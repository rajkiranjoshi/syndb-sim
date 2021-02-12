#ifndef TRIGGERGENERATOR_H
#define TRIGGERGENERATOR_H

#include <functional>
#include "utils/types.hpp"
#include "simulation/config.hpp"

struct triggerInfo {
    sim_time_t triggerOrigTime;
    switch_id_t originSwitch;
    std::map<switch_id_t, sim_time_t> rxSwitchTimes;
};

struct triggerScheduleInfo{
    sim_time_t time;
    switch_id_t switchId;
};

struct TriggerGenerator
{
    std::list<triggerScheduleInfo> triggerSchedule;    
    const sim_time_t initialDelay = syndbConfig.triggerInitialDelay; // 10us - for both topologies
    sim_time_t baseIncrement; // set to topology's switchToSwitchOWD
    uint16_t totalTriggers;
    sim_time_t totalExtraTime;
    std::function<int()> getRandomExtraTime;
    
    sim_time_t nextTriggerTime;
    switch_id_t nextSwitchId;
  
    TriggerGenerator(sim_time_t switchToSwitchOWD, uint16_t totalTriggers);
    void generateTrigger();
    void updateNextTrigger();
    void printTriggerSchedule();
};


struct TriggerGeneratorSimpleTopo: TriggerGenerator
{    
    TriggerGeneratorSimpleTopo();
};


struct TriggerGeneratorFatTreeTopo : TriggerGenerator
{
    TriggerGeneratorFatTreeTopo();
};



#endif
