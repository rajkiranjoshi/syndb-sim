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
}

void testRingBufferOps(){
    switch_p s0 = syndbSim.topo->getSwitchById(0);

    for(int i=1; i <= 8; i++){
        s0->ringBuffer.insertPrecord(i, i); 
    }

    s0->ringBuffer.printActualRingBuffer(10); 
}

void addTriggerPkts(){

    if(syndbConfig.topoType == TopologyType::Simple){    
        const sim_time_t increment = 3000;
        static sim_time_t nextSendTime = 0;

        switch_p srcSwitch = syndbSim.topo->getSwitchById(0); 

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


void testNormalPktLatencies(host_id_t h0, host_id_t h1){

    debug_print_yellow("\n\n#### Packet Latencies (id latency) ####");
    auto it = syndbSim.NormalPktLatencyMap.begin();
    std::list<sim_time_t> h0_send, h0_receive, h1_send, h1_receive;

    for(it; it != syndbSim.NormalPktLatencyMap.end(); it++){
        host_id_t src, dst;
        pkt_id_t id;
        sim_time_t start, end;
        
        id = it->first;
        start = it->second.start_time;
        end = it->second.end_time;
        src = it->second.src;
        dst = it->second.dst;

        // Printing out the latencies for all delivered packets
        if (end != 0){
            debug_print("{}\t{}", id, end-start);

            // For checking inter-pkt gaps at the sending and receiving hosts
            if(src == h0 && dst == h1){ // if src is h0, dst MUST be h1
                h0_send.push_back(start);
                h1_receive.push_back(end);
            }
            else if(src == h1 && dst == h0)
            {
                h1_send.push_back(start);
                h0_receive.push_back(end);
            }

        }
    } // end of iterating over pktLatencyMap

    debug_print_yellow("\nTotal pkts delivered in {} --> {} direction: {}", h0, h1, h0_send.size());
    debug_print_yellow("Total pkts delivered in {} --> {} direction: {}", h1, h0, h1_send.size());

    // Analyzing the inter-pkt gaps at the sender and receiver
    debug_print_yellow("\n\nPacket inter-arrivals in direction {} --> {}", h0, h1);
    auto sendit = h0_send.begin();
    auto receiveit = h1_receive.begin();
    sim_time_t curr_send, curr_receive;
    sim_time_t next_send, next_receive;

    curr_send = *sendit++;
    curr_receive = *receiveit++; 

    while (sendit != h0_send.end())
    {
        next_send = *sendit;
        next_receive = *receiveit;
        debug_print("{}\t{}", next_send-curr_send, next_receive-curr_receive);

        curr_send = next_send;
        curr_receive = next_receive;
        sendit++;
        receiveit++;
    }

    debug_print_yellow("\n\nPacket inter-arrivals in direction {} --> {}", h1, h0);
    sendit = h1_send.begin();
    receiveit = h0_receive.begin();
    curr_send = *sendit++;
    curr_receive = *receiveit++; 
    while (sendit != h1_send.end())
    {
        next_send = *sendit;
        next_receive = *receiveit;
        debug_print("{}\t{}", next_send-curr_send, next_receive-curr_receive);

        curr_send = next_send;
        curr_receive = next_receive;
        sendit++;
        receiveit++;
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
