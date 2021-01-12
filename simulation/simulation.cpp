#include <string>

#include "simulation/config.hpp"
#include "simulation/simulation.hpp"



Simulation::Simulation(){

    this->currTime = 0;
    this->timeIncrement = syndbConfig.timeIncrementNs;
    this->totalTime = syndbConfig.totalTimeMSecs * 1000000;

    if(syndbConfig.topo == "SimpleToplogy"){
        this->topo = SimpleTopology();
    }

    this->nextPktId = 0;

}
/* 
inline pkt_id_t Simulation::getNextPktId(){
    return this->nextPktId++;
} */

Simulation syndbSim;

