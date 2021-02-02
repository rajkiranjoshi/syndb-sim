#include <iostream>

#include "dataparser.hpp"




DataParser::DataParser(std::string prefixStringForFileName, switch_id_t numberOfSwitches, host_id_t numberOfHosts) {

    // open all file pointers in write/output mode
    for (int i = 0; i < numberOfSwitches; i++) {
        std::string fileName = prefixStringForFileName + "switch_" + std::to_string(i) + ".txt";
        std::fstream file ("./data/" + fileName, std::fstream::in);
        this->switchFilePointers.push_back(std::move(file));
    }

    for (int i = 0; i < numberOfHosts; i++) {
        std::string fileName = prefixStringForFileName + "host_" + std::to_string(i) + ".txt";
        std::fstream file ("./data/" + fileName, std::fstream::in);
        this->hostFilePointers.push_back(std::move(file));
    }

}

DataParser::~DataParser() {
    // close all file pointers
    for (int i = 0; i < this->switchFilePointers.size(); i++) {
        this->switchFilePointers[i].close();
    }

    for (int i = 0; i < this->hostFilePointers.size(); i++) {
        this->hostFilePointers[i].close();
    }
}


std::map<sim_time_t, PacketInfo> DataParser::getWindowForSwitch(switch_id_t switchID, sim_time_t triggerTime, pkt_id_t windowSize) {

    this->switchFilePointers[switchID].clear();
    this->switchFilePointers[switchID].seekg(0);

    std::map<sim_time_t, PacketInfo> ingressTimeToPktIDMap;
    std::map<pkt_id_t, PacketInfo> pRecordWindow;

    while (! this->switchFilePointers[switchID].eof()) {
        PacketInfo currentPacket;
        this->switchFilePointers[switchID] >> currentPacket.switchIngressTime >> currentPacket.id >> currentPacket.srcHost >> currentPacket.dstHost;
        // std::cout << currentPacket.switchIngressTime << "\t" << currentPacket.id << "\t" << currentPacket.srcHost << "\t" << currentPacket.dstHost << std::endl;
        
        // store in sorted map <ingressTime, pktID> where ingressTime <= triggerTime
        if (currentPacket.switchIngressTime < triggerTime) {
            ingressTimeToPktIDMap.insert(std::pair<sim_time_t, PacketInfo>(currentPacket.switchIngressTime, currentPacket));
        }
    }

    // from stored index to index-windowSize put all pktID in list/vector
    pkt_id_t numberOfPacketsAddedTopRecordWindow = 0;
    std::map<sim_time_t, PacketInfo>::iterator iteratorForingressTimeToPktIDMap = ingressTimeToPktIDMap.end();
    iteratorForingressTimeToPktIDMap--;     // end()-1 is last element

    while (numberOfPacketsAddedTopRecordWindow < windowSize) {
        // std::cout << iteratorForingressTimeToPktIDMap->first << "\t" << iteratorForingressTimeToPktIDMap->second << std::endl;
        pRecordWindow.insert(std::pair<sim_time_t, PacketInfo>(iteratorForingressTimeToPktIDMap->second.id, iteratorForingressTimeToPktIDMap->second));

        if (iteratorForingressTimeToPktIDMap == ingressTimeToPktIDMap.begin()) {
            break;
        }
        iteratorForingressTimeToPktIDMap--;
        numberOfPacketsAddedTopRecordWindow++;
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