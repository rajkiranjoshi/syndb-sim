#include "simulation/simulation.hpp"
#include "utils/logger.hpp"
#include "devtests/devtests.hpp"

#ifdef DEBUG

void checkRemainingQueuingAtLinks(){
    // Checking queueing on all the links
    auto it1 = syndbSim.topo.torLinkVector.begin();
    debug_print_yellow("nextPktSendTime on ToR links at time {}ns", syndbSim.currTime);
    for (it1; it1 != syndbSim.topo.torLinkVector.end(); it1++){
        debug_print("Link ID {}: towards host: {} | towards tor: {}", (*it1)->id, (*it1)->next_idle_time_to_host, (*it1)->next_idle_time_to_tor);
    }

    auto it2 = syndbSim.topo.networkLinkVector.begin();
    debug_print_yellow("nextPktSendTime on Network links at time {}ns", syndbSim.currTime);
    for (it2; it2 != syndbSim.topo.networkLinkVector.end(); it2++){
        auto map = (*it2)->next_idle_time;

        auto it3 = map.begin();
        switch_id_t sw1 = it3->first;
        sim_time_t dir1 = it3->second;
        it3++;
        switch_id_t sw2 = it3->first;
        sim_time_t dir2 = it3->second;
        ndebug_print("Link ID {}: towards {}: {} | towards {}: {}", (*it2)->id, sw1, dir1, sw2, dir2);
    }
}


void testNormalPktLatencies(host_id_t h0, host_id_t h1){

    debug_print_yellow("\n\n#### Packet Latencies (id latency) ####");
    auto it = syndbSim.pktLatencyMap.begin();
    std::list<sim_time_t> h0_send, h0_receive, h1_send, h1_receive;

    for(it; it != syndbSim.pktLatencyMap.end(); it++){
        host_id_t src, dst;
        pkt_id_t id;
        sim_time_t start, end;
        
        id = it->first;
        start = it->second.start_time;
        end = it->second.end_time;
        src = it->second.srcHost;
        dst = it->second.dstHost;

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



#endif // end of ifdef DEBUG
