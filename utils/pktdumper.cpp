#include <iostream>
#include "utils/logger.hpp"
#include "utils/pktdumper.hpp"


void PktDumper::openFiles(switch_id_t numberOfSwitches, host_id_t numberOfHosts) {
    // create prefix file name consisting of current hour, minute and seconds
    time_t currentTime;
    time(&currentTime);
    const std::tm *calendarTime = std::localtime(&currentTime);
    
    this->prefixStringForFileName = "dump_" + std::to_string(calendarTime->tm_hour) + "_" + 
                                    std::to_string(calendarTime->tm_min) + "_" + 
                                    std::to_string(calendarTime->tm_sec) + "_";
    ndebug_print("PktDump file prefix: {}", this->prefixStringForFileName);

    // open all file pointers in write/output mode
    std::string triggerFileName = "./data/" + prefixStringForFileName + "trigger.txt";
    this->triggerFilePointer.open(triggerFileName, std::fstream::out);

    std::string sourceDestinatoinFileName = "./data/" + prefixStringForFileName + "sourceDestination.txt";
    this->sourceDestinationFilePointer.open(sourceDestinatoinFileName, std::fstream::out);

    debug_print("Number of Switches: {}", numberOfSwitches);
    for (int i = 0; i < numberOfSwitches; i++) {
        std::string fileName = prefixStringForFileName + "switch_" + std::to_string(i) + ".txt";
        std::fstream file ("./data/" + fileName, std::fstream::out);
        this->switchFilePointers.push_back(std::move(file));
    }

    debug_print("Number of Hosts: {}", numberOfHosts);
}


PktDumper::~PktDumper() {
    // close all file pointers
    this->triggerFilePointer.close();
    this->sourceDestinationFilePointer.close();

    for (int i = 0; i < this->switchFilePointers.size(); i++) {
        this->switchFilePointers[i].close();
    }

}


void PktDumper::dumpPacket(normalpkt_p pkt){
    
    debug_print_yellow("--------   HOSTS   ----------");
    debug_print("Start Time: {}", pkt->startTime);
    debug_print("End Time: {}", pkt->endTime);
    this->sourceDestinationFilePointer << pkt->id << "\t" << pkt->srcHost << "\t" << pkt->dstHost << std::endl;

    debug_print_yellow("-------   SWITCHES   ---------");
    std::list<switchINTInfo>::iterator switchTimeStampsIterator;
    for (switchTimeStampsIterator = pkt->switchINTInfoList.begin(); switchTimeStampsIterator != pkt->switchINTInfoList.end(); switchTimeStampsIterator++) {
        this->switchFilePointers[switchTimeStampsIterator->swId] << switchTimeStampsIterator->rxTime << "\t" << pkt->id << std::endl;
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
