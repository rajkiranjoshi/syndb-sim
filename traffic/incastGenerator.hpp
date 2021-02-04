#ifndef INCASTGENERATOR_H
#define INCASTGENERATOR_H

#include <functional>
#include <list>
#include "utils/types.hpp"


struct incastScheduleInfo{
    sim_time_t time;
    host_id_t targetHostId;

    std::list<host_id_t> sourceHosts;
};

struct IncastGenerator
{
    std::list<incastScheduleInfo> incastSchedule;    
    const sim_time_t initialDelay = 10000; // 10us - for both topologies
    
    uint16_t totalIncasts;
    
    sim_time_t nextIncastTime;
  
    IncastGenerator();
    void generateIncast();
    void updateNextIncast();
    void printIncastSchedule();
};


struct IncastGeneratorSimpleTopo: IncastGenerator
{    
    IncastGeneratorSimpleTopo();
};


struct IncastGeneratorFatTreeTopo : IncastGenerator
{
    IncastGeneratorFatTreeTopo();
};



#endif
