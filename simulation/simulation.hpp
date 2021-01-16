#ifndef SIMULATION_H
#define SIMULATION_H

#include <list>
#include <map>
#include "topology/topology.hpp"
#include "simulation/event.hpp"

// For devtest testNormalPktLatencies()
#ifdef DEBUG
typedef struct pktTime
{
    host_id_t srcHost;
    host_id_t dstHost;
    sim_time_t start_time;
    sim_time_t end_time;
} pktTime;
#endif

typedef struct Simulation
{
    Topology topo;

    // time-related. All units are nanoseconds
    sim_time_t currTime;
    sim_time_t timeIncrement;
    sim_time_t totalTime;

    // packets-related
    pkt_id_t nextPktId;
     
    std::list<pktevent_p<normalpkt_p>> NormalPktEventList;
    std::list<pktevent_p<triggerpkt_p>> TriggerPktEventList;

    // For devtest testNormalPktLatencies()
    #ifdef DEBUG
    std::map<pkt_id_t, pktTime> pktLatencyMap;
    #endif

    Simulation(); // default constructor
    
    // needs to be thread-safe when parallelizing
    inline pkt_id_t getNextPktId() { return this->nextPktId++; }; 
    inline void buildTopo(){ this->topo.buildTopo(); };
    void initHosts();
    void processHosts();
    void processTriggerPktEvents();
    void processNormalPktEvents();

    void cleanUp();

} Simulation;

extern Simulation syndbSim;


#endif