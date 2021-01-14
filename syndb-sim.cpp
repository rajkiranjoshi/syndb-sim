#include <stdio.h>
#include <fmt/core.h>
#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"

int main(){

    // Init Step 1: Build the topology
    syndbSim.buildTopo();

    // Init Step 2: Initialize the hosts
    syndbSim.initHosts();

    log_print_info(fmt::format("Running simulation for {}ns ...",syndbSim.totalTime));

    // Main simulation loop
    while (syndbSim.currTime <= syndbSim.totalTime)
    {
        // Step 1: Process all hosts
        syndbSim.processHosts();

        // Step 2: Process all triggerPktEvents
        syndbSim.processTriggerPktEvents();

        // Step 3: Process all normalPktEvents
        syndbSim.processNormalPktEvents();

        // increment the time
        syndbSim.currTime += syndbSim.timeIncrement;
    }
    

    
    log_debug("End of main\n");

    return 0;
}