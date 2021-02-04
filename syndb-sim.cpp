#include <stdio.h>
#include <time.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "devtests/devtests.hpp"

int main(){

    time_t startTime, endTime;
    
    startTime = time(NULL); 

    syndbSim = Simulation();

    // Init Step 1: Build the topology
    syndbSim.buildTopo();
    ndebug_print("Done building topo");

    // Init Step 2: Init the triggerGen schedule
    syndbSim.initTriggerGen();
    syndbSim.triggerGen->printTriggerSchedule();
    
    // Init Step 3: Initialize the hosts
    syndbSim.initHosts();
    ndebug_print("Done init hosts");

    ndebug_print("Running simulation for {}ns ...",syndbSim.totalTime);
    ndebug_print("Time increment is {}", syndbSim.timeIncrement);


    // Main simulation loop: at time = 0; all event lists are empty. Only step 4 does some work.
    for ( ; syndbSim.currTime <= syndbSim.totalTime; syndbSim.currTime += syndbSim.timeIncrement)
    {
        debug_print_yellow("########  Simulation Time: {} ########", syndbSim.currTime); 
        
        // Step 1: Process all hostPktEvents
        syndbSim.processHostPktEvents();
        
        // Step 2: Generate (as per schedule) and process triggerPktEvents
        syndbSim.triggerGen->generateTrigger();
        syndbSim.processTriggerPktEvents();

        // Step 3: Process all normalPktEvents
        syndbSim.processNormalPktEvents();

        // Step 4: Generate hostPktEvents for the next timeIncrement slot
        syndbSim.generateHostPktEvents();

    } // end of main simulation loop

    ndebug_print_yellow("Flushing remaining normal pkts");
    syndbSim.flushRemainingNormalPkts();
    syndbSim.logTriggerInfoMap();
    syndbSim.showLinkUtilizations();

    endTime = time(NULL);
    
#ifdef DEBUG
    checkRemainingQueuingAtLinks();
    // testNormalPktLatencies(0, 1);
    // testRingBufferOps();
    // showSimpleTopoRingBuffers(); 
    // showFatTreeTopoRoutingTables();
#endif

    ndebug_print_yellow("End of main\n");
    ndebug_print_yellow("Simulation run took {} seconds.", endTime - startTime);
    return 0;
}


