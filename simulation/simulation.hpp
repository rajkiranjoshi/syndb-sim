#ifndef SIMULATION_H
#define SIMULATION_H

#include <list>
#include <map>
#include "topology/topology.hpp"
#include "simulation/event.hpp"
#include "utils/pktdumper.hpp"
#include "traffic/triggerGenerator.hpp"
#include "traffic/incastGenerator.hpp"

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
    pkt_id_t totalPktsDelivered;
    
    std::multimap<sim_time_t, hostpktevent_p> HostPktEventList;
    std::list<pktevent_p<normalpkt_p>> NormalPktEventList;
    std::list<pktevent_p<normalpkt_p>> freeNormalPktEvents; // to reuse shared_ptrs
    std::list<pktevent_p<triggerpkt_p>> TriggerPktEventList;

    // For tracking and logging triggerInfo
    std::map<trigger_id_t, triggerInfo> TriggerInfoMap;


    std::shared_ptr<TriggerGenerator> triggerGen;
    std::shared_ptr<IncastGenerator> incastGen;

    std::unique_ptr<PktDumper> pktDumper;

    time_t startTime, endTime;

    Simulation(); // default constructor
    
    // needs to be thread-safe when parallelizing
    inline pkt_id_t getNextPktId() { return this->nextPktId++; }; 
    inline pkt_id_t getNextTriggerPktId() { return this->nextTriggerPktId++; };
    inline void buildTopo(){ this->topo->buildTopo(); };
    pktevent_p<normalpkt_p> getNewNormalPktEvent();
    void initTriggerGen();
    void initIncastGen();
    void initHosts();
    void generateHostPktEvents();
    void processHostPktEvents();
    void processTriggerPktEvents();
    void processNormalPktEvents();
    void flushRemainingNormalPkts();
    void logTriggerInfoMap();
    void showLinkUtilizations();

    void printSimulationStats();
    void cleanUp();

} Simulation;

extern Simulation syndbSim;


#endif