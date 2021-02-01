#ifndef SIMULATION_H
#define SIMULATION_H

#include <list>
#include <map>
#include "topology/topology.hpp"
#include "simulation/event.hpp"
#include "utils/pktdumper.hpp"
#include "traffic/triggerGenerator.hpp"


typedef struct Simulation
{
    std::shared_ptr<Topology> topo;

    // time-related. All units are nanoseconds
    sim_time_t currTime;
    sim_time_t timeIncrement;
    sim_time_t totalTime;

    // packets-related
    pkt_id_t nextPktId;
    pkt_id_t nextTriggerPktId;
     
    std::list<pktevent_p<normalpkt_p>> NormalPktEventList;
    std::list<pktevent_p<triggerpkt_p>> TriggerPktEventList;

    // For tracking and logging triggerInfo
    std::map<trigger_id_t, triggerInfo> TriggerInfoMap;


    std::shared_ptr<TriggerGenerator> triggerGen;

    PktDumper pktDumper;

    Simulation(); // default constructor
    
    // needs to be thread-safe when parallelizing
    inline pkt_id_t getNextPktId() { return this->nextPktId++; }; 
    inline pkt_id_t getNextTriggerPktId() { return this->nextTriggerPktId++; }; 
    inline void buildTopo(){ this->topo->buildTopo(); };
    void initTriggerGen();
    void initHosts();
    void processHosts();
    void processTriggerPktEvents();
    void processNormalPktEvents();
    void flushRemainingNormalPkts();
    void logTriggerInfoMap();
    void showLinkUtilizations();

    void cleanUp();

} Simulation;

extern Simulation syndbSim;


#endif