#include <iostream>
#include <typeinfo>

#include "utils/pktdumper.hpp"
#include "utils/logger.hpp"
#include "traffic/triggerGenerator.hpp"
#include <spdlog/cfg/env.h>



void PktDumper::openFiles(switch_id_t numberOfSwitches, host_id_t numberOfHosts) {
    // create prefix file name consisting of current hour, minute and seconds
    time_t currentTime;
    time(&currentTime);
    const std::tm *calendarTime = std::localtime(&currentTime);

    // clear stpdlog default pattern
    debug_print("Clearing default spdlog dump pattern.");
    spdlog::set_pattern("%v");
    spdlog::init_thread_pool(QUEUE_SIZE, 1);

    
    this->prefixStringForFileName = "dump_" + std::to_string(calendarTime->tm_hour) + "_" + 
                                    std::to_string(calendarTime->tm_min) + "_" + 
                                    std::to_string(calendarTime->tm_sec) + "_";

    std::string simSummaryFileName = "./data/" + prefixStringForFileName + "summary.txt";
    this->simSummaryFilePointer = spdlog::basic_logger_mt("summary", simSummaryFileName);

    // open all file pointers in write/output mode
    std::string triggerFileName = "./data/" + prefixStringForFileName + "trigger.txt";
    this->triggerFilePointer = spdlog::basic_logger_mt<spdlog::async_factory>("trigger", triggerFileName);

    std::string incastFileName = "./data/" + prefixStringForFileName + "incast.txt";
    this->incastFilePointer = spdlog::basic_logger_mt<spdlog::async_factory>("incast", incastFileName);

    std::string sourceDestinationFile = "./data/" + prefixStringForFileName + "sourceDestination.txt";
    this->sourceDestinationFilePointer = spdlog::basic_logger_mt<spdlog::async_factory>("sourceDestination", sourceDestinationFile);

    debug_print("Number of Switches: {}", numberOfSwitches);
    for (int i = 0; i < numberOfSwitches; i++) {
        std::string fileName = "./data/"+ prefixStringForFileName + "switch_" + std::to_string(i) + ".txt";
        auto file = spdlog::basic_logger_mt<spdlog::async_factory>("switch_" + std::to_string(i), fileName);
        this->switchFilePointers.push_back(std::move(file));
    }

    debug_print("Number of Hosts: {}", numberOfHosts);

    ndebug_print("PktDump file prefix: {}", this->prefixStringForFileName);
}


PktDumper::~PktDumper() {
    // close all file pointers
    // this->triggerFilePointer.close();
    // this->sourceDestinationFilePointer.close();

    // for (int i = 0; i < this->switchFilePointers.size(); i++) {
    //     this->switchFilePointers[i].close();
    // }
    // spdlog::shutdown();
}


void PktDumper::dumpPacket(const normalpkt_p pkt){
    
    debug_print_yellow("--------   HOSTS   ----------");
    debug_print("Start Time: {}", pkt->startTime);
    debug_print("End Time: {}", pkt->endTime);
    this->sourceDestinationFilePointer->info("{}\t{}\t{}", pkt->id, pkt->srcHost, pkt->dstHost);

    debug_print_yellow("-------   SWITCHES   ---------");
    std::list<switchINTInfo>::iterator switchTimeStampsIterator;
    for (switchTimeStampsIterator = pkt->switchINTInfoList.begin(); switchTimeStampsIterator != pkt->switchINTInfoList.end(); switchTimeStampsIterator++) {
        this->switchFilePointers[switchTimeStampsIterator->swId]->info("{}\t{}", switchTimeStampsIterator->rxTime, pkt->id);
    }

}


void PktDumper::dumpTriggerInfo(const trigger_id_t &triggerId, triggerInfo &tinfo, const SwitchType &switchType){

    std::string stringForTriggerFile = std::to_string(triggerId) + "\t" + std::to_string(tinfo.triggerOrigTime) + "\t" + std::to_string(tinfo.originSwitch);
    
    std::map<switch_id_t, sim_time_t>::iterator rxSwitchTimesIterator = tinfo.rxSwitchTimes.begin();
    for (; rxSwitchTimesIterator != tinfo.rxSwitchTimes.end(); rxSwitchTimesIterator++) {
        stringForTriggerFile += "\t" + std::to_string(rxSwitchTimesIterator->first) + "\t" + std::to_string(rxSwitchTimesIterator->second);
    }

    this->triggerFilePointer->info("{}", stringForTriggerFile);
}

void PktDumper::dumpIncastInfo(const incastScheduleInfo &incastInfo){
    this->incastFilePointer->info("{}\t{}", incastInfo.time, incastInfo.targetHostId);
}

void PktDumper::logSimSummary(const std::string &msg){
    // this->simSummaryFilePointer->info("{}",msg);
    this->simSummaryFilePointer->info("{}", msg);
}
