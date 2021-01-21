#include <stdio.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "devtests/devtests.hpp"

int main(){

    syndbSim = Simulation();

    // Init Step 1: Build the topology
    syndbSim.buildTopo();
    debug_print("Done building topo");

    debug_print("Time increment is {}", syndbSim.timeIncrement);

    // Init Step 2: Initialize the hosts
    syndbSim.initHosts();
    debug_print("Done init hosts topo");

    debug_print("Running simulation for {}ns ...",syndbSim.totalTime);


    // Main simulation loop
    for ( ; syndbSim.currTime <= syndbSim.totalTime; syndbSim.currTime += syndbSim.timeIncrement)
    {
        debug_print_yellow("########  Simulation Time: {} ########", syndbSim.currTime); 
        
        // Step 1: Process all hosts
        syndbSim.processHosts();
        
        // Step 2: Process all triggerPktEvents
        #ifdef DEBUG
        // addTriggerPkts();
        #endif
        syndbSim.processTriggerPktEvents();

        // Step 3: Process all normalPktEvents
        syndbSim.processNormalPktEvents();

    } // end of main simulation loop

    ndebug_print_yellow("Flushing remaining normal pkts");
    syndbSim.flushRemainingNormalPkts();
    
#ifdef DEBUG
    // checkRemainingQueuingAtLinks();
    // testNormalPktLatencies(0, 1);
    // showTriggerPktLatencies();
    // testRingBufferOps();
    // showSimpleTopoRingBuffers(); 

#endif

    ndebug_print_yellow("End of main\n");
    return 0;
}


