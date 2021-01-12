#include <stdio.h>
#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"

int main(){

    // Build the topology
    syndbSim.topo.buildTopo();
    
    log_debug("End of main\n");

    return 0;
}