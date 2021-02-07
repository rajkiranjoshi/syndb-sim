#include <stdio.h>
#include <signal.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "devtests/devtests.hpp"

void signalHandler(int signum){
    ndebug_print_yellow("\nCaught signal {}", signum);

    syndbSim.cleanUp();

    exit(signum);
}

int main(){

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    syndbSim = Simulation();

    // Init Step 1: Build the topology
    syndbSim.buildTopo();

    // Init Step 2: Init the triggerGen schedule
    #if TRIGGERS_ENABLED
    syndbSim.initTriggerGen();
    syndbSim.triggerGen->printTriggerSchedule();
    #endif
    #if INCASTS_ENABLED
    syndbSim.initIncastGen();
    syndbSim.incastGen->printIncastSchedule();
    #endif
    
    // Init Step 3: Initialize the hosts
    syndbSim.initHosts();

    // Init Step 4: Open files for logging
    #if LOGGING
    syndbSim.pktDumper->openFiles(syndbConfig.numSwitches, syndbConfig.numHosts);
    #endif

    ndebug_print("Time increment is {}ns", syndbSim.timeIncrement);
    ndebug_print("Running simulation for {}ns ...",syndbSim.totalTime);

    // Main simulation loop: at time = 0; all event lists are empty. Only step 4 does some work.
    for ( ; syndbSim.currTime <= syndbSim.totalTime; syndbSim.currTime += syndbSim.timeIncrement)
    {
        debug_print_yellow("########  Simulation Time: {}  ########", syndbSim.currTime);
        if(syndbSim.currTime % 100000 == 0)
            ndebug_print_yellow("########  Simulation Time: {}  ########", syndbSim.currTime);
        
        // Step 1: Process all hostPktEvents
        syndbSim.processHostPktEvents();
        
        // Step 2: Generate (as per schedule) and process triggerPktEvents
        #if TRIGGERS_ENABLED
        syndbSim.triggerGen->generateTrigger();
        syndbSim.processTriggerPktEvents();
        #endif

        // Step 3: Process all normalPktEvents
        syndbSim.processNormalPktEvents();

        // Step 4: Generate (rather modify) (as per schedule) incast pkts on certain hosts
        #if INCASTS_ENABLED
        syndbSim.incastGen->generateIncast(); // these pkts would be picked-up first in step 5
        #endif

        // Step 5: Generate hostPktEvents for the next timeIncrement slot
        syndbSim.generateHostPktEvents();

    } // end of main simulation loop

    syndbSim.cleanUp();

    
#ifdef DEBUG
    checkRemainingQueuingAtLinks();
    // testNormalPktLatencies(0, 1);
    // testRingBufferOps();
    // showSimpleTopoRingBuffers(); 
    // showFatTreeTopoRoutingTables();
#endif

    
    return 0;
}


