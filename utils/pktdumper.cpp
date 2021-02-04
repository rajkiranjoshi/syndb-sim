#include <iostream>
#include "utils/logger.hpp"
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
    ndebug_print("{}", this->prefixStringForFileName);
    #endif


    // open all file pointers in write/output mode
    std::string triggerFileName = "./data/" + prefixStringForFileName + "trigger.txt";
    this->triggerFilePointer.open(triggerFileName, std::fstream::out);

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
    this->triggerFilePointer.close();

    for (int i = 0; i < this->switchFilePointers.size(); i++) {
        this->switchFilePointers[i].close();
    }

    for (int i = 0; i < this->hostFilePointers.size(); i++) {
        this->hostFilePointers[i].close();
    }
}




void PktDumper::dumpPacket(normalpkt_p pkt){
    
    #if DEBUG
    ndebug_print_yellow("--------   HOSTS   ----------");
    ndebug_print("Start Time: {}", pkt->startTime);
    #endif
    this->hostFilePointers[pkt->srcHost] << pkt->startTime << "\t" << pkt->id << std::endl;

    if (pkt->endTime != 0) {
        #if DEBUG
        ndebug_print("End Time: {}", pkt->endTime);
        #endif
        this->hostFilePointers[pkt->dstHost] << pkt->endTime << "\t" << pkt->id << std::endl;
    }

    #if DEBUG
    ndebug_print_yellow("-------   SWITCHES   ---------");
    #endif
    std::list<switchINTInfo>::iterator switchTimeStampsIterator;
    for (switchTimeStampsIterator = pkt->switchINTInfoList.begin(); switchTimeStampsIterator != pkt->switchINTInfoList.end(); switchTimeStampsIterator++) {
        #if DEBUG
        ndebug_print("ID: {} \t Time: {}",switchTimeStampsIterator->swId, switchTimeStampsIterator->rxTime);
        #endif
        this->switchFilePointers[switchTimeStampsIterator->swId] << switchTimeStampsIterator->rxTime << "\t" << pkt->id << "\t";
        this->switchFilePointers[switchTimeStampsIterator->swId] << pkt->srcHost << "\t" << pkt->dstHost << std::endl;
    }

}

void PktDumper::dumpTriggerInfo(trigger_id_t triggerId, triggerInfo tinfo, SwitchType switchType){

    this->triggerFilePointer << triggerId << "\t";
    this->triggerFilePointer << tinfo.triggerOrigTime << "\t" << tinfo.originSwitch;

    std::map<switch_id_t, sim_time_t>::iterator rxSwitchTimesIterator = tinfo.rxSwitchTimes.begin();
    for (; rxSwitchTimesIterator != tinfo.rxSwitchTimes.end(); rxSwitchTimesIterator++) {
        this->triggerFilePointer << "\t" << rxSwitchTimesIterator->first << "\t" << rxSwitchTimesIterator->second;
    }

    this->triggerFilePointer << "\n";
}
