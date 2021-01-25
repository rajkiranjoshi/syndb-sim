#ifndef DEVTESTS_H
#define DEVTESTS_H

#include "utils/types.hpp"
#include "topology/fattree_topology.hpp"

/* 
Iterates over all links (ToR and Network).
For each link, shows next_idle_time in both directions.
Independent of topology. But too many links give cumbersome output.
*/
void checkRemainingQueuingAtLinks();

/* 
Prints OWD between for delivered pkt between two hosts h0 <--> h1. 
Also compares IPG seen at (h0 Tx vs. h1 RX) and (h1 Tx vs. h0 Rx).

Note: Use with small total runtime as it records entry in global 
syndbSim.pktLatencyMap for every generated pkt. 

Ues glue code in Host->generateNextPkt() and Simulation->processNormalPktEvents().
*/
void testNormalPktLatencies(host_id_t h0, host_id_t h1);

/* 
Quick test for shared_ptr destruction.
Independent of any topology.
*/
void testSharedPtrDestruction();

/* 
Periodically adds TriggerPkts to test their latency.
Assumes SimpleTopology.
*/
void addTriggerPkts();

/* 
Prints the latencies of recorded TriggerPkts
*/
void showTriggerPktLatencies();

/* 
Test basic operations of the ring buffer in Isolation of the main simulation
 */
void testRingBufferOps();

/* 
Show the ring buffer states of the 3 switches in SimpleTopo
 */
void showSimpleTopoRingBuffers();

/* 
Prints routing tables of all switches of a FatTree topo
*/
void showFatTreeTopoRoutingTables();
/* {
    
    std::shared_ptr<FattreeTopology> fattreetopo = std::dynamic_pointer_cast<FattreeTopology>(syndbSim.topo); 

    debug_print_yellow("\n-------------  Routing Tables [Core Switches]  -------------");
    auto it = fattreetopo->coreSwitches.begin();
    for(it; it != fattreetopo->coreSwitches.end(); it++){
        std::shared_ptr<SwitchFtCore> coreSwitch = std::dynamic_pointer_cast<SwitchFtCore>(*it);
        
        debug_print("Core Switch {}:", coreSwitch->id);
        auto entry_it = coreSwitch->routingTable.begin();
        for(entry_it; entry_it != coreSwitch->routingTable.end(); entry_it++){
            debug_print("{} --> {}", entry_it->first, entry_it->second);
        }
        debug_print("");
    }

    debug_print_yellow("\n-------------  Routing Tables [Aggr Switches]  -------------");
    for(int i = 0; i < fattreetopo->k; i++){ // iterate over all pods
        for(int j = 0; j < (fattreetopo->k/2); j++){
            std::shared_ptr<SwitchFtAggr> aggrSwitch = std::dynamic_pointer_cast<SwitchFtAggr>(fattreetopo->pods[i]->aggrSwitches[j]);

            debug_print("Aggr Switch {}:", aggrSwitch->id);
            auto entry_it = aggrSwitch->routingTable.begin();
            for(entry_it; entry_it != aggrSwitch->routingTable.end(); entry_it++){
                debug_print("{} --> {}", entry_it->first, entry_it->second);
            }
            debug_print("");
        }
    }

    debug_print_yellow("\n-------------  Routing Tables [ToR Switches]  -------------");
    for(int i = 0; i < fattreetopo->k; i++){ // iterate over all pods
        for(int j = 0; j < (fattreetopo->k/2); j++){
            std::shared_ptr<SwitchFtTor> torSwitch = std::dynamic_pointer_cast<SwitchFtTor>(fattreetopo->pods[i]->torSwitches[j]);

            debug_print("ToR Switch {}:", torSwitch->id);
            auto entry_it = torSwitch->routingTable.begin();
            for(entry_it; entry_it != torSwitch->routingTable.end(); entry_it++){
                debug_print("{} --> {}", entry_it->first, entry_it->second);
            }
            debug_print("");
        }
    }
    
}
*/

#endif
