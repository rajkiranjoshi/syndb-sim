#ifndef INCASTGENERATOR_H
#define INCASTGENERATOR_H

#include <memory>
#include <functional>
#include <list>
#include <set>
#include "utils/types.hpp"


struct incastScheduleInfo{
    sim_time_t time;
    host_id_t targetHostId;

    std::set<host_id_t> sourceHosts;

    void printScheduleInfo();
};

typedef std::shared_ptr<incastScheduleInfo> incastScheduleInfo_p;

struct IncastGenerator
{
    std::list<incastScheduleInfo_p> incastSchedule;    
    const sim_time_t initialDelay = 10000; // 10us - for both topologies
    
    uint16_t totalIncasts;
    
    sim_time_t nextIncastTime;
    incastScheduleInfo_p nextIncast;
  
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
