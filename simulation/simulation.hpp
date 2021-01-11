#ifndef SIMULATION_H
#define SIMULATION_H

#include <list>
#include "topology/topology.hpp"

typedef struct Simulation
{
    Topology topo;

    // time-related. All units are nanoseconds
    sim_time_t currTime;
    sim_time_t timeIncrement;
    sim_time_t totalTime;

    // packets-related
    pkt_id_t nextPktId;
    std::list<normalpkt_p> NormalPktList;
    std::list<triggerpkt_p> TriggerPktList;
    
    
    Simulation(); // default constructor
    pkt_id_t getNextPktId(); // needs to be thread-safe when parallelizing

} Simulation;

extern Simulation syndbSim;


#endif