#include <iostream>
#include "utils/pktdumper.hpp"


PktDumper::PktDumper(switch_id_t numberOfSwitches, host_id_t numberOfHosts) {

    // create prefix file name consisting of current hour, minute and seconds
    time_t currentTime;
    time(&currentTime);
    const std::tm *calendarTime = std::localtime(&currentTime);
    
    this->prefixStringForFileName = "dump_" + std::to_string(calendarTime->tm_hour) + "_" + 
                                    std::to_string(calendarTime->tm_min) + "_" + 
                                    std::to_string(calendarTime->tm_sec) + "_";
    #ifdef DEBUG
    std::cout<< this->prefixStringForFileName << std::endl;
    #endif


    // open all file pointers in write/output mode
    for (int i = 0; i < numberOfSwitches; i++) {
        std::string fileName = prefixStringForFileName + "switch_" + std::to_string(i) + ".txt";
        std::fstream file ("./data/" + fileName, std::fstream::out);
        this->switchFilePointers.push_back(std::move(file));
    }

    for (int i = 0; i < numberOfHosts; i++) {
        std::string fileName = prefixStringForFileName + "host_" + std::to_string(i) + ".txt";
        std::fstream file ("./data/" + fileName, std::fstream::out);
        this->hostFilePointers.push_back(std::move(file));
    }
}


PktDumper::~PktDumper() {
    // close all file pointers
    for (int i = 0; i < this->switchFilePointers.size(); i++) {
        this->switchFilePointers[i].close();
    }

    for (int i = 0; i < this->hostFilePointers.size(); i++) {
        this->hostFilePointers[i].close();
    }
}




void PktDumper::dumpPacket(normalpkt_p pkt){
    
    #if DEBUG
    std::cout<<"--------   HOSTS   ----------"<< std::endl;
    std::cout<<"Start Time: " << pkt->startTime << std::endl;
    #endif
    this->hostFilePointers[pkt->srcHost] << pkt->startTime << "\t" << pkt->id << std::endl;

    if (pkt->endTime != 0) {
        #if DEBUG
        std::cout<<"End Time: "<< pkt->endTime << std::endl;
        #endif
        this->hostFilePointers[pkt->dstHost] << pkt->endTime << "\t" << pkt->id << std::endl;
    }

    #if DEBUG
    std::cout<<"-------   SWITCHES   ---------"<< std::endl;
    #endif
    std::list<switchINTInfo>::iterator switchTimeStampsIterator;
    for (switchTimeStampsIterator = pkt->switchINTInfoList.begin(); switchTimeStampsIterator != pkt->switchINTInfoList.end(); switchTimeStampsIterator++) {
        #if DEBUG
        std::cout<<"ID: " << switchTimeStampsIterator->swId;
        std::cout<<"\tTime: " << switchTimeStampsIterator->rxTime << std::endl;
        #endif
        this->switchFilePointers[switchTimeStampsIterator->swId] << switchTimeStampsIterator->rxTime << "\t" << pkt->id << "\t";
        this->switchFilePointers[switchTimeStampsIterator->swId] << pkt->srcHost << "\t" << pkt->dstHost << std::endl;
    }

}

void PktDumper::dumpTriggerInfo(trigger_id_t triggerId, triggerInfo tinfo, SwitchType switchType){

}
