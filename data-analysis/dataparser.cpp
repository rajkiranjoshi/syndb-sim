#include <iostream>

#include "data-analysis/dataparser.hpp"
#include "utils/logger.hpp"


DataParser::DataParser(std::string prefixFilePath, std::string prefixStringForFileName, switch_id_t numberOfSwitches, host_id_t numberOfHosts) {

    std::string pathForDataFolder = prefixFilePath + "/data/" + prefixStringForFileName;

    // open all file pointers in write/output mode
    for (int i = 0; i < numberOfSwitches; i++) {
        std::string fileName = pathForDataFolder + "switch_" + std::to_string(i) + ".txt";
        std::fstream file (fileName, std::fstream::in);
        this->switchFilePointers.push_back(std::move(file));
    }

    this->triggerFilePointer.open(pathForDataFolder + "trigger.txt", std::fstream::in);
    this->sourceDestinationFilePointer.open(pathForDataFolder + "sourceDestination.txt", std::fstream::in);

}

DataParser::~DataParser() {
    // close all file pointers
    for (int i = 0; i < this->switchFilePointers.size(); i++) {
        this->switchFilePointers[i].close();
    }

    this->triggerFilePointer.close();
    this->sourceDestinationFilePointer.close();

}


std::map<pkt_id_t, PacketInfo> DataParser::getWindowForSwitch(switch_id_t switchID, sim_time_t triggerTime, pkt_id_t windowSize, bool isTriggerSwitch) {

    this->switchFilePointers[switchID].clear();
    this->switchFilePointers[switchID].seekg(0);

    std::multimap<sim_time_t, PacketInfo> ingressTimeToPktIDMap;
    std::map<pkt_id_t, PacketInfo> pRecordWindow;

    while (! this->switchFilePointers[switchID].eof()) {
        PacketInfo currentPacket;
        this->switchFilePointers[switchID] >> currentPacket.switchIngressTime >> currentPacket.id;
        // debug_print("{}\t{}", currentPacket.switchIngressTime, currentPacket.id);
        
        // store in sorted map <ingressTime, PacketInfo> where ingressTime <= triggerTime
        if (currentPacket.switchIngressTime <= triggerTime) {
            ingressTimeToPktIDMap.insert(std::pair<sim_time_t, PacketInfo>(currentPacket.switchIngressTime, currentPacket));
        }
    }

    // from stored index to index-windowSize put all pktID in list/vector
    pkt_id_t numberOfPacketsAddedTopRecordWindow = 0;
    std::multimap<sim_time_t, PacketInfo>::iterator iteratorForingressTimeToPktIDMap = ingressTimeToPktIDMap.end();
    iteratorForingressTimeToPktIDMap--;     // end()-1 is last element

    while (numberOfPacketsAddedTopRecordWindow < windowSize+5) {
        debug_print("{}\t{}", iteratorForingressTimeToPktIDMap->first, iteratorForingressTimeToPktIDMap->second.id);
        if (numberOfPacketsAddedTopRecordWindow < windowSize)
            pRecordWindow.insert(std::pair<sim_time_t, PacketInfo>(iteratorForingressTimeToPktIDMap->second.id, iteratorForingressTimeToPktIDMap->second));

        if (iteratorForingressTimeToPktIDMap == ingressTimeToPktIDMap.begin()) {
            break;
        }
        iteratorForingressTimeToPktIDMap--;
        numberOfPacketsAddedTopRecordWindow++;
    }

    pkt_id_t packetId;
    host_id_t source, destination;
    if (isTriggerSwitch) {
        this->sourceDestinationFilePointer.clear();
        this->sourceDestinationFilePointer.seekg(0);
        while (! this->sourceDestinationFilePointer.eof()) {
            this->sourceDestinationFilePointer >> packetId >> source >> destination;

            auto positionInMap = pRecordWindow.find(packetId);
            if (positionInMap != pRecordWindow.end()) {
                positionInMap->second.srcHost = source;
                positionInMap->second.dstHost = destination;
            }
        }
    }

    return pRecordWindow;
}



float DataParser::getCorrelationBetweenPrecordWindows(std::map<pkt_id_t, PacketInfo> precordWindowForTriggerSwitch, std::map<pkt_id_t, PacketInfo> precordWindowForCurrentSwitch) {

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
}

void DataParser::getTriggerInfo(switch_id_t numberOfSwitches) {

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
}