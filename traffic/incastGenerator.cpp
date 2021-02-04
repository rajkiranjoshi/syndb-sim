#include <cassert>
#include <set>
#include <random>
#include <chrono>
#include <algorithm>    // std::random_shuffle
#include "traffic/incastGenerator.hpp"
#include "simulation/config.hpp"

IncastGenerator::IncastGenerator(){

    assert(syndbConfig.incastFanInRatio <= syndbConfig.numHosts && "Incast fan-in ratio must NOT be larger than total numHosts!");

    sim_time_t totalTime = (sim_time_t)(syndbConfig.totalTimeMSecs * (float)1000000);

    sim_time_t totalTicks = totalTime / syndbConfig.timeIncrementNs;

    sim_time_t fullUtilTicksNeeded = ((double)syndbConfig.percentIncastTime / 100.0) * (double) totalTicks;

    /* ASSUMPTION: 
       Each incast pkt is small enough and timeIncrement is big enough such that in one timeIncrement, one incast pkt can serialize over the target host's ToR link.

       syndbConfig.percentIncastTime we want *each* target link to face queuing.

       Time for which one incast keeps a link fully utilized = syndbConfig.incastFanInRatio (# of time ticks)
    */
    sim_time_t fullUtilTicksPerIncast = syndbConfig.incastFanInRatio;

    sim_time_t incastsPerTargetLink = fullUtilTicksNeeded / fullUtilTicksPerIncast;

    host_id_t numTargetHosts = ((double)syndbConfig.percentTargetIncastHosts / 100.0) * syndbConfig.numHosts;

    sim_time_t totalIncasts = numTargetHosts * incastsPerTargetLink;

    /* Scheduling the incasts over time */
    sim_time_t availableTime = totalTime - this->initialDelay;


    /* Pick randomly numTargetHosts number of unique hosts */
    std::set<host_id_t> targetHosts;
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine randTargetHosts(seed);

    while (targetHosts.size() != numTargetHosts)
    {
        host_id_t host = randTargetHosts() % syndbConfig.numHosts;
        targetHosts.insert(host);
    }

    std::vector<host_id_t> targetHostsVector;

    for(auto it = targetHosts.begin(); it != targetHosts.end(); it++){
        host_id_t host = *it;
        for(int i=0; i < incastsPerTargetLink; i++){
            targetHostsVector.push_back(host);
        }
    }
    std::random_shuffle(targetHostsVector.begin(), targetHostsVector.end()); 

    /* TODO: Generate the schedule and add hosts from targetHostsVector sequentially */
    



}
