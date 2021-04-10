#include <iostream>
#include <limits.h>

#include "data-analysis/dataparser.hpp"

#define LOGGING 0
#include "utils/logger.hpp"


/**
 * Execute a shell command passed to the function
 */
std::string DataParser::executeShellCommand(const char* command) {
    std::array<char, 512> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

/**
 * Create and open file pointers for all data files.
 */
DataParser::DataParser(std::string prefixFilePath, std::string prefixStringForFileName, switch_id_t numberOfSwitches) {

    std::string pathForDataFolder = prefixFilePath + "/" + prefixStringForFileName +"/" + prefixStringForFileName;
    ndebug_print_yellow("Reading files {}*.txt.", pathForDataFolder);
    // open all file pointers in write/output mode
    for (int i = 0; i < numberOfSwitches; i++) {
        std::string fileName = pathForDataFolder + "_switch_" + std::to_string(i) + ".txt";
        std::fstream file (fileName, std::fstream::in);
        this->switchFilePointers.push_back(std::move(file));
    }

    this->triggerFilePointer.open(pathForDataFolder + "_trigger.txt", std::fstream::in);
    this->sourceDestinationFilePointer.open(pathForDataFolder + "_sourceDestination.txt", std::fstream::in);

}


DataParser::~DataParser() {
    // close all file pointers
    for (int i = 0; i < this->switchFilePointers.size(); i++) {
        this->switchFilePointers[i].close();
    }

    this->triggerFilePointer.close();
    this->sourceDestinationFilePointer.close();

}


/**
 * Get the p-record window for a switch at a particular point of time.
 *
 * @param triggerTime   Time the switch received/generated the trigger packet
 * @param windowSize    Size of the p-record window
 * @param isTriggerSwitch   Boolean to denote if the switch is trigger switch.
 *                          If the switch is trigger switch we get the host and destination
 *                          for each packet in its trigger window.
 *
 */
std::map<pkt_id_t, PacketInfo> DataParser::getWindowForSwitch(switch_id_t switchID, sim_time_t triggerTime, pkt_id_t windowSize, bool isTriggerSwitch) {

    std::map<pkt_id_t, PacketInfo> pRecordWindow;

    // get line number of last packet before triggerTime
    std::string prefixFilePath = PREFIX_FILE_PATH;
    std::string prefixStringForFileName = PREFIX_STRING_FOR_DATA_FILES;
    std::string pathForDataFolder = prefixFilePath + "/" + prefixStringForFileName + "/" + prefixStringForFileName;
    std::string fileName = pathForDataFolder + "_switch_" + std::to_string(switchID) + ".txt";
    std::string prefixForCommandToGetLineNumber = "cat " + fileName + " | cut -f 1 |" + "grep -n -w -F ";
    std::string suffixForCommandToGetLineNumber = " | cut -d \":\" -f 1";
    uint64_t startLineNumber = 0;
    uint64_t skipBytes = 0;

    while (triggerTime > 0) {

        std::string commandToGetLineNumber = prefixForCommandToGetLineNumber + std::to_string(triggerTime) + suffixForCommandToGetLineNumber;
        std::string lineNumber = this->executeShellCommand(commandToGetLineNumber.c_str());

        if (lineNumber.size() != 0) {
            startLineNumber = std::stoll(lineNumber);
            break;
        }
        triggerTime--;
    }

#ifdef DEBUG
    debug_print("Starting line number is: {}", startLineNumber);
    debug_print("Starting trigger time is: {}", triggerTime);
#endif
    std::string commandToGetTime = "sed -n " + std::to_string(startLineNumber+100000) + "p " + fileName + " | cut -f 1";
    sim_time_t end_time_100k = std::stoll(this->executeShellCommand(commandToGetTime.c_str()));
    sim_time_t end_time_5M = triggerTime;
    commandToGetTime = "sed -n " + std::to_string(startLineNumber+5000000) + "p " + fileName + " | cut -f 1";
    std::string timeString = this->executeShellCommand(commandToGetTime.c_str());
    if (timeString.size() != 0) {
        end_time_5M = std::stoll(timeString);
    }
    ndebug_print("{}\t{}\t{}", switchID, triggerTime-end_time_100k, triggerTime-end_time_5M);
    return pRecordWindow;

    this->switchFilePointers[switchID].clear();
    this->switchFilePointers[switchID].seekg(0);
    for (uint64_t currLineNumber = 0; currLineNumber < startLineNumber - 1; ++currLineNumber){
        sim_time_t tempIngressTime;

        if (this->switchFilePointers[switchID].ignore(std::numeric_limits<std::streamsize>::max(), this->switchFilePointers[switchID].widen('\n'))){ 
            // skip till the line before start of pRecord window
        }
    }

    pkt_id_t numberOfPacketsAddedTopRecordWindow = 0;
    pkt_id_t smallestPktID = 0, largestPktID = 0;
    
    while (numberOfPacketsAddedTopRecordWindow < windowSize && ! this->switchFilePointers[switchID].eof()) {
        PacketInfo currentPacket;
        this->switchFilePointers[switchID] >> currentPacket.switchIngressTime >> currentPacket.id;
        pRecordWindow.insert(std::pair<pkt_id_t, PacketInfo>(currentPacket.id, currentPacket));

        if (smallestPktID == 0 || smallestPktID > currentPacket.id) {
            smallestPktID = currentPacket.id;
        } 
        if (largestPktID < currentPacket.id) {
            largestPktID = currentPacket.id;
        }

        numberOfPacketsAddedTopRecordWindow++;
    }

    debug_print("pRecord Window Size for Switch ID {} : {}", switchID, numberOfPacketsAddedTopRecordWindow);

#ifdef DEBUG
    auto it = pRecordWindow.begin();
    debug_print("--- pRecord window for switch {} ---", switchID);
    while (it != pRecordWindow.end()) {
        debug_print("{}\t{}", it->first, it->second.switchIngressTime);
        it++;
    }
#endif

    debug_print("SUCCESS: Obtained pRecord Window for {}.", switchID);

    if (isTriggerSwitch) {

        // triggerTime now contains the tstamp of the most recent packet before the trigger packet is received
        sim_time_t timeOfMostRecentpRecord = triggerTime; 
        // the next line to read in the file contains the packet most recently removed from the pRecord window
        sim_time_t timeOfLeastRecentpRecord = 0;
        this->switchFilePointers[switchID] >> timeOfLeastRecentpRecord;
        sim_time_t historyRecordedInpRecordWindow = 0;
        if (timeOfLeastRecentpRecord < timeOfMostRecentpRecord) {
            historyRecordedInpRecordWindow = timeOfMostRecentpRecord - timeOfLeastRecentpRecord - 1;
        } else {
            historyRecordedInpRecordWindow = timeOfMostRecentpRecord - 1;
        }
        ndebug_print("Trigger Switch pRecord Window History {}ns", historyRecordedInpRecordWindow);
        ndebug_print("Smallest pkt ID: {}\t Largest pkt ID:{}", smallestPktID, largestPktID);

        // skip lines in sourceDestination file
        
        fileName = pathForDataFolder + "_sourceDestination.txt";
        prefixForCommandToGetLineNumber = "LC_ALL=C grep -m 1 -b -w -F ";
        std::string commandToGetSkipBytes = prefixForCommandToGetLineNumber + std::to_string(smallestPktID) + " " + fileName +suffixForCommandToGetLineNumber;
        debug_print("{}", commandToGetSkipBytes);
        std::string skipBytesString = this->executeShellCommand(commandToGetSkipBytes.c_str());
        skipBytes = std::stoll(skipBytesString);
        debug_print("Skip Bytes for sourceDestination file: {}", skipBytes);        

        startLineNumber = smallestPktID - 1; // pkt ID starts from 0
        this->sourceDestinationFilePointer.clear();
        this->sourceDestinationFilePointer.seekg(skipBytes);

        pkt_id_t packetId = 0;
        host_id_t source, destination;
        long numberOfCompletedpRecords = 0;
        auto pRecordIterator = pRecordWindow.begin();
        while (packetId <= largestPktID) {
            this->sourceDestinationFilePointer >> packetId >> source >> destination;
            
            // auto positionInMap = pRecordWindow.find(packetId);
            if (pRecordIterator->first == packetId) {
                debug_print("{}", packetId);

                pRecordIterator->second.srcHost = source;
                pRecordIterator->second.dstHost = destination;
                numberOfCompletedpRecords++;
                
                pRecordIterator++;
                skipBytes = this->sourceDestinationFilePointer.tellg();
                skipBytes += (pRecordIterator->first - packetId) * 21;
                this->sourceDestinationFilePointer.seekg(skipBytes);
                if (this->sourceDestinationFilePointer.ignore(std::numeric_limits<std::streamsize>::max(), this->sourceDestinationFilePointer.widen('\n'))){ 
                    // skip till the line before start of pRecord window
                }
                
#ifdef DEBUG
                if (numberOfCompletedpRecords >= windowSize) {
                    break;
                    ndebug_print("Found source and destination for all packets in the precord window.");
                }
#endif
            } else if (pRecordIterator->first < packetId) {
                skipBytes -= (packetId - pRecordIterator->first) * 21;
                this->sourceDestinationFilePointer.seekg(skipBytes);
                if (this->sourceDestinationFilePointer.ignore(std::numeric_limits<std::streamsize>::max(), this->sourceDestinationFilePointer.widen('\n'))){ 
                    // skip till the line before start of pRecord window
                }
            }
        }
    }

    return pRecordWindow;
}


/**
 * Get information regarding triggers from the trigger dump file.
 */
void DataParser::getTriggerInfo(switch_id_t numberOfSwitches) {
    ndebug_print("Reading Trigger File.");

    while (! this->triggerFilePointer.eof()) {
        TriggerInfo trigger;
        this->triggerFilePointer >> trigger.triggerId >> trigger.triggerTime >> trigger.originSwitch;        
        if (this->triggerFilePointer.eof()) {
            break;
        }
        debug_print_yellow("Trigger ID: {}\t Switch: {}\t Time: {}", trigger.triggerId, trigger.triggerTime, trigger.originSwitch);

        for (int i = 0; i < numberOfSwitches-1; i++) {
            sim_time_t timeOfReceivingTriggerPacket;
            switch_id_t switchID;
            this->triggerFilePointer >> switchID >> timeOfReceivingTriggerPacket;
            debug_print("\t Switch: {}\t Time: {}", switchID, timeOfReceivingTriggerPacket);
            trigger.mapOfSwitchTriggerTime.insert(std::pair<switch_id_t, sim_time_t>(switchID, timeOfReceivingTriggerPacket));
        }
        this->listOfTriggers.push_back(trigger);
    }

    ndebug_print_yellow("Read {} triggers.", this->listOfTriggers.size());
}