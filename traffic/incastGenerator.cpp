#include <cassert>
#include <set>
#include <random>
#include <chrono>
#include <algorithm>    // std::random_shuffle
#include "traffic/incastGenerator.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"
#include "topology/host.hpp"

IncastGenerator::IncastGenerator(){

    assert(syndbConfig.incastFanInRatio <= syndbConfig.numHosts && "Incast fan-in ratio must NOT be larger than total numHosts!");

    sim_time_t totalTime = (sim_time_t)(syndbConfig.totalTimeMSecs * (float)1000000);

    sim_time_t fullUtilTimeNeeded = ((double)syndbConfig.percentIncastTime / 100.0) * (double) totalTime;

    /* ASSUMPTION:
       We are going to change/override packet sizes to 1500B. Time for which one incast keeps a link fully utilized = serialization time of 1550B pkt x syndbConfig.incastFanInRatio
    */
    sim_time_t fullUtilTimePerIncast = syndbConfig.incastFanInRatio * getSerializationDelay(1500, syndbConfig.torLinkSpeedGbps); 
    sim_time_t incastsPerTargetLink = fullUtilTimeNeeded / fullUtilTimePerIncast;

    // Handle the case that simulation time is much small such that fullUtilTimeNeeded > fullUtilTimePerIncast
    // Guarantee that at least 1 incast per target link
    incastsPerTargetLink = std::max<sim_time_t>(incastsPerTargetLink, 1);

    host_id_t numTargetHosts = ((double)syndbConfig.percentTargetIncastHosts / 100.0) * syndbConfig.numHosts;
    sim_time_t totalIncasts = numTargetHosts * incastsPerTargetLink;
    this->totalIncasts = totalIncasts;
/* 
    // Single test trigger
    incastScheduleInfo_p newIncast = std::shared_ptr<incastScheduleInfo>(new incastScheduleInfo());
    newIncast->time = 25000;
    newIncast->targetHostId = 0;
    for(int i=1; i < syndbConfig.numHosts; i++){
        newIncast->sourceHosts.push_back(i);
    }
    this->incastSchedule.push_back(newIncast);
*/
    // Here OnWard: Scheduling the incasts over time
    sim_time_t availableTime = totalTime - this->initialDelay;

    // Pick randomly numTargetHosts number of unique hosts
    std::set<host_id_t> targetHosts;
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine randTargetHosts(seed);

    while (targetHosts.size() != numTargetHosts)
    {
        host_id_t host = randTargetHosts() % syndbConfig.numHosts;
        targetHosts.insert(host);
    }

    // Fill the vector with repeated host IDs
    std::vector<host_id_t> targetHostsVector;
    for(auto it = targetHosts.begin(); it != targetHosts.end(); it++){
        host_id_t host = *it;
        for(int i=0; i < incastsPerTargetLink; i++){
            targetHostsVector.push_back(host);
        }
    }
    std::random_shuffle(targetHostsVector.begin(), targetHostsVector.end()); 

    // Generate the schedule and add hosts from targetHostsVector sequentially
    sim_time_t interIncastGap = availableTime / (totalIncasts + 1);
    sim_time_t halfSimTimeIncrement = syndbSim.timeIncrement / 2;
    
    // Align the interIncastGap to sim timeIncr
    interIncastGap = ((interIncastGap + halfSimTimeIncrement) / syndbSim.timeIncrement) * syndbSim.timeIncrement;

    seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine randSourceHosts(seed);
    sim_time_t currTime = this->initialDelay + interIncastGap;
    host_id_t srcHost;

    for(auto it = targetHostsVector.begin(); it != targetHostsVector.end(); it++){
       
        incastScheduleInfo_p newIncast = std::shared_ptr<incastScheduleInfo>(new incastScheduleInfo()); 
        newIncast->time = currTime;
        newIncast->targetHostId = *it;

        while (newIncast->sourceHosts.size() != syndbConfig.incastFanInRatio)
        {
            srcHost = randSourceHosts() % syndbConfig.numHosts;
            if(srcHost != newIncast->targetHostId)
                newIncast->sourceHosts.insert(srcHost);
        }
        
        this->incastSchedule.push_back(newIncast);

        currTime += interIncastGap; // update for next incast
    }
    // update nextIncast pointers from the schedule
    this->updateNextIncast();
}


void IncastGenerator::updateNextIncast(){
    if(this->incastSchedule.size() > 0){
        this->nextIncast = *this->incastSchedule.begin();
        this->nextIncastTime = this->nextIncast->time;
        this->incastSchedule.pop_front();
    }
}

void IncastGenerator::generateIncast(){

    if(syndbSim.currTime == this->nextIncastTime){ // it MUST be equal

        host_id_t targetHost = this->nextIncast->targetHostId;

        for(auto it = this->nextIncast->sourceHosts.begin(); it != this->nextIncast->sourceHosts.end(); it++){
            Host* host = syndbSim.topo->getHostById(*it);

            host->nextPkt->size = 1500;
            host->nextPkt->dstHost = targetHost;

            sim_time_t newNextPktTime = host->prevPktTime + getSerializationDelay(1500, syndbConfig.torLinkSpeedGbps);
            host->nextPktTime = newNextPktTime;
            host->torLink->next_idle_time_to_tor = newNextPktTime;

            // ndebug_print("Incast pkt ID: {}", host->nextPkt->id);
        }

        syndbSim.pktDumper->dumpIncastInfo(*this->nextIncast);

        this->updateNextIncast();
    }
    
}

void IncastGenerator::printIncastSchedule(){
    
    ndebug_print_yellow("Incast Schedule: {} total incasts", this->totalIncasts);
    
    // Print the lined-up incast first
    this->nextIncast->printScheduleInfo();

    // Then print the rest

    for(auto it = this->incastSchedule.begin(); it != this->incastSchedule.end(); it++){
        (*it)->printScheduleInfo();
    }

}

void incastScheduleInfo::printScheduleInfo(){
    #ifdef DEBUG
    host_id_t targetHost = this->targetHostId;
    sim_time_t incastTime = this->time;
    std::string srcHosts = "";
    for(auto it = this->sourceHosts.begin(); it != this->sourceHosts.end(); it++){
        srcHosts.append(fmt::format("{} ", *it));            
    }
    debug_print("Time: {}\tTarget: {} | Sources: {}", incastTime, targetHost, srcHosts); 
    #endif
}
