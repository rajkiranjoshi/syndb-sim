#include "simulation/simulation.hpp"
#include "utils/logger.hpp"
#include "devtests/devtests.hpp"

#ifdef DEBUG

void showFatTreeTopoRoutingTables(){
    
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
        }
    }
    
}


void showSimpleTopoRingBuffers(){
    #if RING_BUFFER
    switch_p s0, s1, s2;
    
    s0 = syndbSim.topo->getSwitchById(0);
    s1 = syndbSim.topo->getSwitchById(1);
    s2 = syndbSim.topo->getSwitchById(2);

    debug_print_yellow("\nRing buffer for s0:");
    s0->ringBuffer.printRingBuffer();

    debug_print_yellow("\nRing buffer for s1:");
    s1->ringBuffer.printRingBuffer();

    debug_print_yellow("\nRing buffer for s2:");
    s2->ringBuffer.printRingBuffer();

    debug_print_yellow("\nActual Ring buffer for s0:");
    s0->ringBuffer.printActualRingBuffer(5);

    debug_print_yellow("\nActual Ring buffer for s1:");
    s1->ringBuffer.printActualRingBuffer(5);

    debug_print_yellow("\nActual Ring buffer for s2:");
    s2->ringBuffer.printActualRingBuffer(5);
    #endif
}

void testRingBufferOps(){
    #if RING_BUFFER
    switch_p s0 = syndbSim.topo->getSwitchById(0);

    for(int i=1; i <= 8; i++){
        s0->ringBuffer.insertPrecord(i, i); 
    }

    s0->ringBuffer.printActualRingBuffer(10); 
    #endif
}

void addTriggerPkts(){

    if(syndbConfig.topoType == TopologyType::Simple){    
        const sim_time_t increment = 3000;
        static sim_time_t nextSendTime = 0;

        Switch* srcSwitch = syndbSim.topo->getSwitchById(0); 

        if(syndbSim.currTime >= nextSendTime){

            srcSwitch->generateTrigger(); 

            nextSendTime += increment;
        }
    }
    else if(syndbConfig.topoType == TopologyType::FatTree){
        static bool generatedAlready = false;

        if(generatedAlready == false){
            // Generate a single trigger on switchID 2
            syndbSim.topo->getSwitchById(9)->generateTrigger();
            generatedAlready = true;
        }
    }

}

void checkRemainingQueuingAtLinks(){
    // Checking queueing on all the links
    auto it1 = syndbSim.topo->torLinkVector.begin();
    debug_print_yellow("nextPktSendTime on ToR links at time {}ns", syndbSim.currTime);
    for (it1; it1 != syndbSim.topo->torLinkVector.end(); it1++){
        debug_print("Link ID {}: towards host: {} | towards tor: {}", (*it1)->id, (*it1)->next_idle_time_to_host, (*it1)->next_idle_time_to_tor);
    }

    auto it2 = syndbSim.topo->networkLinkVector.begin();
    debug_print_yellow("nextPktSendTime on Network links at time {}ns", syndbSim.currTime);
    for (it2; it2 != syndbSim.topo->networkLinkVector.end(); it2++){
        auto map = (*it2)->next_idle_time;

        auto it3 = map.begin();
        switch_id_t sw1 = it3->first;
        sim_time_t dir1 = it3->second;
        it3++;
        switch_id_t sw2 = it3->first;
        sim_time_t dir2 = it3->second;
        ndebug_print("Link ID {} Normal: towards {}: {} | towards {}: {}", (*it2)->id, sw1, dir1, sw2, dir2);

        auto mapPriority = (*it2)->next_idle_time_priority;
        auto it4 = mapPriority.begin();
        sw1 = it4->first;
        dir1 = it4->second;
        it4++;
        sw2 = it4->first;
        dir2 = it4->second;
        ndebug_print("Link ID {} Priority: towards {}: {} | towards {}: {}", (*it2)->id, sw1, dir1, sw2, dir2);
    }
}



void testSharedPtrDestruction(){

    // Create and add packetEvents
    normalpkt_p p1, p2;
    p1 = normalpkt_p(new NormalPkt(1, 1500));
    p2 = normalpkt_p(new NormalPkt(2, 1500));

    pktevent_p<normalpkt_p> newPktEvent1 = pktevent_p<normalpkt_p>(new PktEvent<normalpkt_p>());
    pktevent_p<normalpkt_p> newPktEvent2 = pktevent_p<normalpkt_p>(new PktEvent<normalpkt_p>());

    newPktEvent1->pkt = p1;
    newPktEvent2->pkt = p2;

    syndbSim.NormalPktEventList.push_back(newPktEvent1);
    syndbSim.NormalPktEventList.push_back(newPktEvent2);    

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

#endif // end of DEBUG
