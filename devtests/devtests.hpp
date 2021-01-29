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

#endif
