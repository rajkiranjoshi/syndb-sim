#include <random>
#include <functional>
#include "simulation/simulation.hpp"
#include "traffic/triggerGenerator.hpp"
#include "utils/logger.hpp"
#include "simulation/config.hpp"



TriggerGenerator::TriggerGenerator(sim_time_t switchToSwitchOWD, uint16_t totalTriggers){
    // assumes the constructor will be called after the topo is built

    this->baseIncrement = switchToSwitchOWD;
    this->totalTriggers = totalTriggers;

    // compute feasibility of triggers
    sim_time_t totalTime = (sim_time_t)(syndbConfig.totalTimeMSecs * (float)1000000);
    sim_time_t availableTime = totalTime - this->initialDelay;

    uint16_t maxPossibleTriggers = (availableTime / this->baseIncrement) - 1;

    if(totalTriggers > maxPossibleTriggers){
        std::string msg = fmt::format("Cannot place {} triggers for total sim time of {}ms. Max triggers possible for this sim time are {}.", totalTriggers, syndbConfig.totalTimeMSecs, maxPossibleTriggers);
        throw std::logic_error(msg);
    }

    sim_time_t totalExtraTime = availableTime - ((this->totalTriggers + 1) * this->baseIncrement);
    sim_time_t extraTimePerTrigger = totalExtraTime / this->totalTriggers;
    std::default_random_engine generator(std::random_device{}());
    std::uniform_int_distribution<int> extraTimeDist(0, extraTimePerTrigger);
    this->getRandomExtraTime = std::bind(extraTimeDist, generator);

}

void TriggerGenerator::generateTrigger(){

    if(this->nextTriggerTime ==  syndbSim.currTime){ // it MUST be equal
        switch_p currSwitch = syndbSim.topo->getSwitchById(this->nextSwitchId);
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


TriggerGeneratorSimpleTopo::TriggerGeneratorSimpleTopo():TriggerGenerator::TriggerGenerator(2012, syndbConfig.numTriggersSimpleTopo){  

    sim_time_t halfSimTimeIncrement = syndbSim.timeIncrement / 2;
    sim_time_t currBaseTime = this->initialDelay + this->baseIncrement;
    sim_time_t nextTime;
    switch_id_t nextSwitch;

    std::default_random_engine generator(std::random_device{}());
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
    ndebug_print_yellow("Showing schedule for {} triggers", this->triggerSchedule.size());

    auto it = this->triggerSchedule.begin();

    for(it; it != this->triggerSchedule.end(); it++){
        ndebug_print("{} {}", it->time, it->switchId);
    }

}


TriggerGeneratorFatTreeTopo::TriggerGeneratorFatTreeTopo():TriggerGenerator::TriggerGenerator(4024, syndbConfig.numTriggersSimpleTopo){  


}
