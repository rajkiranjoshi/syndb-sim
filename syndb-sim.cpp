#include <stdio.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"

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
        syndbSim.processTriggerPktEvents();

        // Step 3: Process all normalPktEvents
        syndbSim.processNormalPktEvents();

        // increment the time
        // syndbSim.currTime += syndbSim.timeIncrement;
    }
    
    // Checking queueing on all the links
    auto it1 = syndbSim.topo.torLinkVector.begin();
    ndebug_print_yellow("nextPktSendTime on ToR links at time {}ns", syndbSim.currTime);
    for (it1; it1 != syndbSim.topo.torLinkVector.end(); it1++){
        ndebug_print("{} {}", (*it1)->next_idle_time_to_host, (*it1)->next_idle_time_to_tor);
    }

    auto it2 = syndbSim.topo.networkLinkVector.begin();
    ndebug_print_yellow("nextPktSendTime on Network links at time {}ns", syndbSim.currTime);
    for (it2; it2 != syndbSim.topo.networkLinkVector.end(); it2++){
        auto map = (*it2)->next_idle_time;

        auto it3 = map.begin();
        switch_id_t sw1 = it3->first;
        sim_time_t dir1 = it3->second;
        it3++;
        switch_id_t sw2 = it3->first;
        sim_time_t dir2 = it3->second;
        ndebug_print("towards {}: {} | towards {}: {}", sw1, dir1, sw2, dir2);
        // ndebug_print("{}", map);
    }

    ndebug_print_yellow("Next pkt number: {}", syndbSim.nextPktId);

    ndebug_print_yellow("End of main\n");

    return 0;
}


void createAddPkts(){
    normalpkt_p p1, p2;
    p1 = normalpkt_p(new NormalPkt(1, 1500));
    p2 = normalpkt_p(new NormalPkt(2, 1500));

    pktevent_p<normalpkt_p> newPktEvent1 = pktevent_p<normalpkt_p>(new PktEvent<normalpkt_p>());
    pktevent_p<normalpkt_p> newPktEvent2 = pktevent_p<normalpkt_p>(new PktEvent<normalpkt_p>());

    newPktEvent1->pkt = p1;
    newPktEvent2->pkt = p2;

    syndbSim.NormalPktEventList.push_back(newPktEvent1);
    syndbSim.NormalPktEventList.push_back(newPktEvent2);
}

void testSharedPtrDestruction(){
     // Quick test for shared_ptr destruction
    debug_print("Before createAddPkts!");
    createAddPkts();
    getchar();
    debug_print("After createAddPkts!");

    std::list<std::list<pktevent_p<normalpkt_p>>::iterator> toDelete;

    auto it = syndbSim.NormalPktEventList.begin();

    while(it != syndbSim.NormalPktEventList.end() ){
        debug_print(fmt::format("Found pkt with id {} inside PktEvent", (*it)->pkt->id));
        toDelete.push_back(it);

        it++;
    }

    debug_print(fmt::format("Deleting {} NormalPktEvents...", toDelete.size()));
    auto it2 = toDelete.begin();

    while (it2 != toDelete.end()){
        syndbSim.NormalPktEventList.erase(*it2);
        it2++;
    }
}
