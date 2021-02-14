#include <iostream>
#include <limits.h>

#include "data-analysis/dataparser.hpp"

#define LOGGING 0
#include "utils/logger.hpp"


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


DataParser::DataParser(std::string prefixFilePath, std::string prefixStringForFileName, switch_id_t numberOfSwitches, host_id_t numberOfHosts) {

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


std::unordered_map<pkt_id_t, PacketInfo> DataParser::getWindowForSwitch(switch_id_t switchID, sim_time_t triggerTime, pkt_id_t windowSize, bool isTriggerSwitch) {

    std::unordered_map<pkt_id_t, PacketInfo> pRecordWindow;

    // get line number of last packet before triggerTime
    std::string prefixFilePath = PREFIX_FILE_PATH;
    std::string prefixStringForFileName = PREFIX_STRING_FOR_DATA_FILES;
    std::string pathForDataFolder = prefixFilePath + "/" + prefixStringForFileName + "/" + prefixStringForFileName;
    std::string fileName = pathForDataFolder + "_switch_" + std::to_string(switchID) + ".txt";
    std::string prefixForCommandToGetLineNumber = "cat " + fileName + " | cut -f 1 |" + "grep -n -w ";
    std::string suffixForCommandToGetLineNumber = " | cut -d \":\" -f 1";
    uint64_t startLineNumber = 0;

    while (triggerTime > 0) {

        std::string commandToGetLineNumber = prefixForCommandToGetLineNumber + std::to_string(triggerTime) + suffixForCommandToGetLineNumber;
        debug_print("Executing command: {}", commandToGetLineNumber);
        std::string lineNumber = this->executeShellCommand(commandToGetLineNumber.c_str());
        debug_print("Line number of time {} is {}", triggerTime, lineNumber);

        if (lineNumber.size() != 0) {
            startLineNumber = std::stoll(lineNumber);
            break;
        }
        triggerTime--;
    }

    debug_print("Starting line number is: {}", startLineNumber);
    this->switchFilePointers[switchID].clear();
    this->switchFilePointers[switchID].seekg(0);
    for (uint64_t currLineNumber = 0; currLineNumber < startLineNumber - 1; ++currLineNumber){
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
    

        // auto firstEntryInpRecordWindow = pRecordWindow.begin();
        // auto lastEntryInpRecordWindow = pRecordWindow.end();
        // lastEntryInpRecordWindow--;
        // pkt_id_t smallestPktID = firstEntryInpRecordWindow->first;
        // pkt_id_t largestPktID = lastEntryInpRecordWindow->first;
        ndebug_print("Smallest pkt ID: {}\t Largest pkt ID:{}", smallestPktID, largestPktID);

        // skip lines in sourceDestination file
        
        fileName = pathForDataFolder + "_sourceDestination.txt";
        prefixForCommandToGetLineNumber = "grep -m 1 -b -w ";
        std::string commandToGetSkipBytes = prefixForCommandToGetLineNumber + std::to_string(smallestPktID) + " " + fileName +suffixForCommandToGetLineNumber;
        debug_print("{}", commandToGetSkipBytes);
        std::string lineNumber = this->executeShellCommand(commandToGetSkipBytes.c_str());
        uint64_t skipBytes = std::stoll(lineNumber);
        debug_print("Skip Bytes for sourceDestination file: {}", skipBytes);        

        startLineNumber = smallestPktID - 1; // pkt ID starts from 0
        this->sourceDestinationFilePointer.clear();
        this->sourceDestinationFilePointer.seekg(skipBytes);

/*         while (true) {
            pkt_id_t tempPacketID;
            this->sourceDestinationFilePointer.seekg(skipBytes);
            if (this->sourceDestinationFilePointer.ignore(std::numeric_limits<std::streamsize>::max(), this->sourceDestinationFilePointer.widen('\n'))){ 
                // skip till the line before start of pRecord window
            }
            this->sourceDestinationFilePointer >> tempPacketID;
            if (tempPacketID < smallestPktID) {
                skipBytes += (smallestPktID - tempPacketID) * 8;
            } else if (smallestPktID < tempPacketID) {
                skipBytes -= (tempPacketID - smallestPktID) * 8;
            }
        } */

        pkt_id_t packetId = 0;
        host_id_t source, destination;
        long numberOfCompletedpRecords = 0;
        
        while (packetId <= largestPktID) {
            this->sourceDestinationFilePointer >> packetId >> source >> destination;
            
            auto positionInMap = pRecordWindow.find(packetId);
            if (positionInMap != pRecordWindow.end()) {
                debug_print("{}", packetId);

                positionInMap->second.srcHost = source;
                positionInMap->second.dstHost = destination;
                numberOfCompletedpRecords++;

                #ifdef DEBUG
                if (numberOfCompletedpRecords >= windowSize) {
                    break;
                    ndebug_print("Found source and destination for all packets in the precord window.");
                }
                #endif
            }
        }
    }

    return pRecordWindow;
}



/* float DataParser::getCorrelationBetweenPrecordWindows(std::map<pkt_id_t, PacketInfo> precordWindowForTriggerSwitch, std::map<pkt_id_t, PacketInfo> precordWindowForCurrentSwitch) {

    pkt_id_t numberOfCommonPktIDs = 0, totalNumberOfExpectedPackets = 0;
    std::map<pkt_id_t, PacketInfo>::iterator iteratorForprecordWindowForTriggerSwitch = precordWindowForTriggerSwitch.begin();
    std::map<pkt_id_t, PacketInfo>::iterator iteratorForprecordWindowForCurrentSwitch;

    while (iteratorForprecordWindowForTriggerSwitch != precordWindowForTriggerSwitch.end()) {

        // get route for packet from src and dst 

        // if switch in route then
        totalNumberOfExpectedPackets++;

            iteratorForprecordWindowForCurrentSwitch = precordWindowForCurrentSwitch.find(iteratorForprecordWindowForTriggerSwitch->first);

            if (iteratorForprecordWindowForCurrentSwitch != precordWindowForCurrentSwitch.end()) {
                // std::cout << "Commont Entry for packet ID [" << iteratorForprecordWindowForTriggerSwitch->first << "] : ";
                // std::cout << "{" << iteratorForprecordWindowForTriggerSwitch->second << "}" << " {" << iteratorForprecordWindowForCurrentSwitch->second << "}" << std::endl;
                numberOfCommonPktIDs++;
            }
            iteratorForprecordWindowForTriggerSwitch++;
    }

    std::cout<< "Common Pkts: " << numberOfCommonPktIDs << std::endl;
    return (float)numberOfCommonPktIDs / (float)totalNumberOfExpectedPackets;
} */

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