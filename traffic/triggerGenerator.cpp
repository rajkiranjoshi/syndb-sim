#include <random>
#include <chrono>
#include <functional>
#include <algorithm>    // std::random_shuffle
#include "simulation/simulation.hpp"
#include "traffic/triggerGenerator.hpp"
#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "topology/fattree_topology.hpp"



TriggerGenerator::TriggerGenerator(sim_time_t switchToSwitchOWD, uint16_t totalTriggers){
    // assumes the constructor will be called after the topo is built

    this->baseIncrement = switchToSwitchOWD;
    this->totalTriggers = totalTriggers;

    // compute feasibility of triggers
    sim_time_t totalTime = (sim_time_t)(syndbConfig.totalTimeMSecs * (float)1000000);
    if(this->initialDelay >= totalTime){
        std::string msg = fmt::format("Total sim time of {}ms <= initial trigger delay of {}ms. Fix syndbConfig.triggerInitialDelay and/or syndbConfig.totalTimeMSecs", syndbConfig.totalTimeMSecs, (double)this->initialDelay/1000000.0);
        throw std::logic_error(msg);
    }

    sim_time_t availableTime = totalTime - this->initialDelay;

    uint16_t maxPossibleTriggers = (availableTime / this->baseIncrement) - 1;

    if(totalTriggers > maxPossibleTriggers){
        std::string msg = fmt::format("Cannot place {} triggers for total sim time of {}ms. Max triggers possible for this sim time are {}.", totalTriggers, syndbConfig.totalTimeMSecs, maxPossibleTriggers);
        throw std::logic_error(msg);
    }

    sim_time_t totalExtraTime = availableTime - ((this->totalTriggers + 1) * this->baseIncrement);
    sim_time_t extraTimePerTrigger = totalExtraTime / this->totalTriggers;
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> extraTimeDist((extraTimePerTrigger * 9 / 10), extraTimePerTrigger);
    this->getRandomExtraTime = std::bind(extraTimeDist, generator);

}

void TriggerGenerator::generateTrigger(){

    if(this->nextTriggerTime ==  syndbSim.currTime){ // it MUST be equal
        Switch* currSwitch = syndbSim.topo->getSwitchById(this->nextSwitchId);
        currSwitch->generateTrigger(); 

        this->updateNextTrigger();
    }

}

void TriggerGenerator::updateNextTrigger(){
    static auto nextTrigger = this->triggerSchedule.begin();

    this->nextTriggerTime = nextTrigger->time;
    this->nextSwitchId    = nextTrigger->switchId;

    nextTrigger++;
}

/* 
    Constructs the trigger schedule for Simple topo. Then calls updateNextTrigger() so that nextTriggerTime and nextSwitchId are initialized.
*/
TriggerGeneratorSimpleTopo::TriggerGeneratorSimpleTopo():TriggerGenerator::TriggerGenerator(2012, syndbConfig.numTriggersPerSwitchType){  

    sim_time_t halfSimTimeIncrement = syndbSim.timeIncrement / 2;
    sim_time_t currBaseTime = this->initialDelay + this->baseIncrement;
    sim_time_t nextTime;
    switch_id_t nextSwitch;

    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> switchSelectDist(0, syndbSim.topo->nextSwitchId-1);
    auto getRandomSwitchId = std::bind(switchSelectDist, std::ref(generator));

    for(int i=0; i < this->totalTriggers; i++){
        triggerScheduleInfo newScheduleInfo;

        nextSwitch = getRandomSwitchId();
        nextTime = currBaseTime + this->getRandomExtraTime();
        // round nextTime to nearest simulator increment
        nextTime = ((nextTime + halfSimTimeIncrement) / syndbSim.timeIncrement) * syndbSim.timeIncrement;

        newScheduleInfo.time = nextTime;
        newScheduleInfo.switchId = nextSwitch;

        this->triggerSchedule.push_back(newScheduleInfo);

        // update currBaseTime
        // currBaseTime += this->baseIncrement;
        currBaseTime = nextTime + this->baseIncrement;
    }

    this->updateNextTrigger();
}


void TriggerGenerator::printTriggerSchedule(){
    ndebug_print_yellow("Trigger Schedule: {} total triggers", this->triggerSchedule.size());

    auto it = this->triggerSchedule.begin();

    for(it; it != this->triggerSchedule.end(); it++){
        debug_print("{} {}", it->time, it->switchId);
    }

}

/* 
    Constructs the trigger schedule for FatTree topo. Then calls updateNextTrigger() so that nextTriggerTime and nextSwitchId are initialized.
*/
TriggerGeneratorFatTreeTopo::TriggerGeneratorFatTreeTopo():TriggerGenerator::TriggerGenerator(4024, 3 * syndbConfig.numTriggersPerSwitchType){  

    std::shared_ptr<FattreeTopology> fatTreeTopo = std::dynamic_pointer_cast<FattreeTopology>(syndbSim.topo);

    std::vector<switch_id_t> coreSwitches(fatTreeTopo->switchTypeIDMap[SwitchType::FtCore].begin(),fatTreeTopo->switchTypeIDMap[SwitchType::FtCore].end()); 
    std::vector<switch_id_t> aggrSwitches(fatTreeTopo->switchTypeIDMap[SwitchType::FtAggr].begin(),fatTreeTopo->switchTypeIDMap[SwitchType::FtAggr].end()); 
    std::vector<switch_id_t> torSwitches(fatTreeTopo->switchTypeIDMap[SwitchType::FtTor].begin(),fatTreeTopo->switchTypeIDMap[SwitchType::FtTor].end());

    // debug_print_yellow("Initialized vectors of switchID");
    // debug_print("Total switches: {} core, {} aggr, {} tor", coreSwitches.size(), aggrSwitches.size(), torSwitches.size());

    sim_time_t halfSimTimeIncrement = syndbSim.timeIncrement / 2;
    sim_time_t currBaseTime = this->initialDelay + this->baseIncrement;
    sim_time_t nextTime, randomExtraTime;
    switch_id_t nextSwitch;

    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> torSelectDist(0, torSwitches.size()-1);
    std::uniform_int_distribution<int> aggrSelectDist(0, aggrSwitches.size()-1);
    std::uniform_int_distribution<int> coreSelectDist(0, coreSwitches.size()-1);

    auto getRandomTorSwitchIdx = std::bind(torSelectDist, generator);
    auto getRandomAggrSwitchIdx = std::bind(aggrSelectDist, generator);
    auto getRandomCoreSwitchIdx = std::bind(coreSelectDist, generator);

    std::vector<switch_id_t> nextSwitchIds;

    for(int i=0; i < syndbConfig.numTriggersPerSwitchType; i++)
        nextSwitchIds.push_back(torSwitches[getRandomTorSwitchIdx()]);

    for(int i=0; i < syndbConfig.numTriggersPerSwitchType; i++)
        nextSwitchIds.push_back(aggrSwitches[getRandomAggrSwitchIdx()]);

    for(int i=0; i < syndbConfig.numTriggersPerSwitchType; i++)
        nextSwitchIds.push_back(coreSwitches[getRandomCoreSwitchIdx()]);

    std::random_shuffle(nextSwitchIds.begin(), nextSwitchIds.end());
    
    for(int i=0; i < nextSwitchIds.size(); i++){
        
        triggerScheduleInfo newScheduleInfo;
        
        nextSwitch = nextSwitchIds[i]; 
        randomExtraTime = this->getRandomExtraTime();
        nextTime = currBaseTime + randomExtraTime;
        // round nextTime to nearest simulator increment
        nextTime = ((nextTime + halfSimTimeIncrement) / syndbSim.timeIncrement) * syndbSim.timeIncrement;

        newScheduleInfo.time = nextTime;
        newScheduleInfo.switchId = nextSwitch;

        this->triggerSchedule.push_back(newScheduleInfo);

        // update currBaseTime
        // currBaseTime += this->baseIncrement;
        currBaseTime = nextTime + this->baseIncrement;
    }

    this->updateNextTrigger();

}
