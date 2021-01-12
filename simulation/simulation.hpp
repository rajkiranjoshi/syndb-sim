#ifndef SIMULATION_H
#define SIMULATION_H

#include <list>
#include "topology/topology.hpp"
#include "simulation/event.hpp"

typedef struct Simulation
{
    Topology topo;

    // time-related. All units are nanoseconds
    sim_time_t currTime;
    sim_time_t timeIncrement;
    sim_time_t totalTime;

    // packets-related
    pkt_id_t nextPktId;
    std::list<normalpktevent_p> NormalPktEventList;
    std::list<triggerpktevent_p> TriggerPktEventList;
    
    
    Simulation(); // default constructor
    
    // needs to be thread-safe when parallelizing
    inline pkt_id_t getNextPktId() {return this->nextPktId++;}; 

} Simulation;

extern Simulation syndbSim;


#endif