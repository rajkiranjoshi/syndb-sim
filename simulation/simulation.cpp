#include <string>
#include <fmt/core.h>

#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "utils/logger.hpp"
#include "utils/utils.hpp"
#include "topology/fattree_topology.hpp"


Simulation syndbSim;

Simulation::Simulation(){

    this->currTime = 0;
    this->timeIncrement = syndbConfig.timeIncrementNs;
    this->totalTime = (sim_time_t)(syndbConfig.totalTimeMSecs * (float)1000000);

    if(syndbConfig.topoType == TopologyType::Simple){
        this->topo = std::shared_ptr<Topology>(new SimpleTopology());
    }
    else if (syndbConfig.topoType == TopologyType::FatTree){
        this->topo = std::shared_ptr<Topology>(new FattreeTopology(syndbConfig.fatTreeTopoK));
    }

    this->nextPktId = 0;
    this->nextTriggerPktId = 0;

    #if LOGGING
    this->pktDumper = std::unique_ptr<PktDumper>(new PktDumper());
    #endif
    
}

void Simulation::initTriggerGen(){
    switch(syndbConfig.topoType){
        case TopologyType::Simple:
            this->triggerGen = std::shared_ptr<TriggerGenerator>(new TriggerGeneratorSimpleTopo());
            break;
        case TopologyType::FatTree:
            this->triggerGen = std::shared_ptr<TriggerGenerator>(new TriggerGeneratorFatTreeTopo());
            break;
    }
}

void Simulation::initIncastGen(){
    this->incastGen = std::shared_ptr<IncastGenerator>(new IncastGenerator());
}

void Simulation::initHosts(){

    auto it = this->topo->hostIDMap.begin();

    while (it != this->topo->hostIDMap.end() )
    {
        host_p h = it->second;

        if(syndbConfig.trafficPatternType == TrafficPatternType::FtMixed){
            std::dynamic_pointer_cast<FtMixedTrafficPattern>(h->trafficPattern)->initTopoInfo();
        }

        h->generateNextPkt();

        it++;
    }

    debug_print(fmt::format("Initialized {} hosts!", this->topo->hostIDMap.size()));
    
}

void Simulation::generateHostPktEvents(){
    
    // HostPktEventList MUST be empty
    assert((this->HostPktEventList.size() == 0) && "HostPktEventList is NOT empty!");

    for(auto it = this->topo->hostIDMap.begin(); it != this->topo->hostIDMap.end(); it++){
        host_p host = it->second;

        #ifdef DEBUG
        // Just for debug case when trafficGen is disabled
        if (host->trafficGenDisabled)
            continue;
            
        #endif
        
        while(host->nextPktTime <= syndbSim.currTime + syndbSim.timeIncrement){
            // Use the next scheduled packet on the host to create hostPktEvent
            hostpktevent_p hostPktEvent = hostpktevent_p(new HostPktEvent(host, host->nextPkt));

            // Insert the hostPktEvent into the map (sorted list)
            this->HostPktEventList.insert(std::pair<sim_time_t, hostpktevent_p>(host->nextPktTime,hostPktEvent));

            // Generate next scheduled packet on the host
            host->generateNextPkt();
        }

    }

}

void Simulation::processHostPktEvents(){

    normalpkt_p nextPkt;
    sim_time_t nextPktTime;
    host_p host;

    auto it = this->HostPktEventList.begin();

    while (it != this->HostPktEventList.end() )
    {
        nextPktTime = it->first;
        host = it->second->host;
        nextPkt = it->second->pkt;

        if(this->currTime < nextPktTime){
            std::string msg = fmt::format("Currtime: {}ns. HostPktEventList has pkt with nextPktTime {}ns", this->currTime, nextPktTime);
            throw std::logic_error(msg);
        }

        host->sendPkt(nextPkt, nextPktTime);

        it = this->HostPktEventList.erase(it); // erase and increment iterator

    }
}



void Simulation::processTriggerPktEvents(){
    
    routeScheduleInfo rsinfo;

    // List of iterators that we would delete in the end
    std::list<std::list<pktevent_p<triggerpkt_p>>::iterator> toDelete;

    toDelete.clear(); // Making sure that the list is empty

    auto it = this->TriggerPktEventList.begin();

    while (it != this->TriggerPktEventList.end())
    {
        pktevent_p<triggerpkt_p> event = *it;

        if(this->currTime >= event->pktForwardTime){

            // Pass the pkt to the next switch to handle
            event->nextSwitch->receiveTriggerPkt(event->pkt, event->pktForwardTime); // can parallelize switch's processing? 

            // Handling the case that the next hop is the pkt's dstSwitch
            if(event->nextSwitch->id == event->pkt->dstSwitchId){
                // The pkt requires no more network event processing
                // Add the event's iterator to the toDelete list
                toDelete.push_front(it); 
            }
            else // forward the event's packet
            {
                // Call routing on the next switch
                syndb_status_t status = event->nextSwitch->routeScheduleTriggerPkt(event->pkt, event->pktForwardTime, rsinfo);

                if(status != syndb_status_t::success){
                    std::string msg = fmt::format("Simulator failed to route trigger pkt of trigger event {}", event->pkt->triggerId);
                    throw std::logic_error(msg);
                }

                // event->doForwarding(rinfo);

                event->currSwitch = event->nextSwitch;
                event->nextSwitch = rsinfo.nextSwitch;
                event->pktForwardTime = rsinfo.pktNextForwardTime;
                
            }                
        }

        it++;  // iterating over TriggerPktEventList
    }

    // Now delete the enlisted TriggerPktEvents

    // debug_print(fmt::format("Deleting {} TriggerPktEvents...", toDelete.size()));
    auto it2 = toDelete.begin();

    while (it2 != toDelete.end()){
        TriggerPktEventList.erase(*it2);

        it2++;
    }

    
}


void Simulation::processNormalPktEvents(){

    routeScheduleInfo rsinfo;

    // List of iterators that we would delete in the end
    std::list<std::list<pktevent_p<normalpkt_p>>::iterator> toDelete;

    toDelete.clear(); // Making sure that the list is empty

    auto it = this->NormalPktEventList.begin();

    while (it != this->NormalPktEventList.end())
    {
        pktevent_p<normalpkt_p> event = *it;

        if(this->currTime >= event->pktForwardTime){

            
            // Handling the case that the next hop is the dst Host
            if(event->nextSwitch == NULL){
                // Mark the event for deletion
                toDelete.push_back(it);
                

                // Add end time INT data to the packet
                event->pkt->endTime = event->pktForwardTime;

                // Dump the pkt with INT data to the disk
                #if LOGGING
                syndbSim.pktDumper->dumpPacket(event->pkt);
                #endif

                #ifdef DEBUG
                debug_print_yellow("\nPkt ID {} dump:", event->pkt->id);
                debug_print("h{} --> h{}: {} ns (Start: {} ns | End: {} ns)", event->pkt->srcHost, event->pkt->dstHost, event->pkt->endTime - event->pkt->startTime, event->pkt->startTime, event->pkt->endTime);
                auto it1 = event->pkt->switchINTInfoList.begin();
                for(it1; it1 != event->pkt->switchINTInfoList.end(); it1++){
                    debug_print("Rx on s{} at {} ns", it1->swId, it1->rxTime);
                }
                #endif
            }
            // Handling the case that the next hop is a switch (intermediate or dstTor)
            else{
                
                // Pass the pkt to the next switch to handle
                event->nextSwitch->receiveNormalPkt(event->pkt, event->pktForwardTime); // can parallelize switch's processing?

                // Call routing on the next switch
                syndb_status_t status = event->nextSwitch->routeScheduleNormalPkt(event->pkt, event->pktForwardTime, rsinfo);
                
                
                /* Update the event */
                event->currSwitch = event->nextSwitch;
                event->nextSwitch = rsinfo.nextSwitch; // will be NULL if next hop is a host
                event->pktForwardTime = rsinfo.pktNextForwardTime;               

            }

        } // end of the if(this->currTime >= event->pktForwardTime)

        it++;  // iterating over NormalPktEventList
    } 

    // Now delete the enlisted NormalPktEvents

    // debug_print(fmt::format("Deleting {} NormalPktEvents...", toDelete.size()));
    auto it2 = toDelete.begin();

    while (it2 != toDelete.end()){
        NormalPktEventList.erase(*it2);
        it2++;
    }


}


void Simulation::flushRemainingNormalPkts(){

    auto it = this->NormalPktEventList.begin();

    for(it; it != this->NormalPktEventList.end(); it++){
        pktevent_p<normalpkt_p> event = *it;

        // Dump the pkt with INT data to the disk
        #if LOGGING
        syndbSim.pktDumper->dumpPacket(event->pkt);
        #endif


        #ifdef DEBUG
        /* debug_print_yellow("\nPkt ID {} dump:", event->pkt->id);
        debug_print("h{} --> h{}: N/A ns (Start: {} ns | End: {} ns)", event->pkt->srcHost, event->pkt->dstHost, event->pkt->startTime, event->pkt->endTime);
        auto it1 = event->pkt->switchINTInfoList.begin();
        for(it1; it1 != event->pkt->switchINTInfoList.end(); it1++){
            debug_print("Rx on s{} at {} ns", it1->swId, it1->rxTime);
        } */
        #endif
    }

}

void Simulation::logTriggerInfoMap(){
    sim_time_t triggerOriginTime, rxTime; 
    trigger_id_t triggerId;
    switch_id_t originSwitch, rxSwitch;

    auto it1 = syndbSim.TriggerInfoMap.begin();

    ndebug_print_yellow("\nTrigger pkt latencies between switches");
    for(it1; it1 != syndbSim.TriggerInfoMap.end(); it1++){
        
        triggerId = it1->first;
        originSwitch = it1->second.originSwitch;
        SwitchType switchType = syndbSim.topo->getSwitchTypeById(originSwitch);

        #if LOGGING
        syndbSim.pktDumper->dumpTriggerInfo(triggerId, it1->second, switchType);
        #else
        /* Below code is only for debugging. TODO: comment out later. */

        /*
        triggerOriginTime = it1->second.triggerOrigTime;
        
        ndebug_print_yellow("Trigger ID {} (origin switch: {} {})", triggerId, originSwitch, switchTypeToString(switchType));
        auto it2 = it1->second.rxSwitchTimes.begin();

        for(it2; it2 != it1->second.rxSwitchTimes.end(); it2++){
            rxSwitch = it2->first;
            rxTime = it2->second;

            ndebug_print("{} --> {}: {}ns", originSwitch, rxSwitch, rxTime - triggerOriginTime);
        } // end of iterating over rxSwitchTimes
         */
        #endif


    } // end of iterating over TriggerPktLatencyMap
}

void Simulation::showLinkUtilizations(){

    double util_to_tor, util_to_host;
    double util1, util2;
    double percent_util1, percent_util2;
    double percent_util_to_tor, percent_util_to_host;
    double torLinksPercentUtilSum = 0;
    double networkLinksPercentUtilSum = 0;
    link_id_t numTorLinks = 0; 
    link_id_t numNetworkLinks = 0; 

    debug_print_yellow("Utilization on ToR links:");
    for(auto it = syndbSim.topo->torLinkVector.begin(); it != syndbSim.topo->torLinkVector.end(); it++){
        
        util_to_tor = (double)((*it)->byte_count_to_tor * 8) / syndbSim.totalTime;
        util_to_host = (double)((*it)->byte_count_to_host * 8) / syndbSim.totalTime;

        percent_util_to_tor = (util_to_tor / syndbConfig.torLinkSpeedGbps) * 100.0;
        percent_util_to_host = (util_to_host / syndbConfig.torLinkSpeedGbps) * 100.0;

        debug_print("Link ID {}: towards host: {} | towards tor: {}", (*it)->id, percent_util_to_host, percent_util_to_tor);

        torLinksPercentUtilSum += percent_util_to_tor;
        torLinksPercentUtilSum += percent_util_to_host;
        numTorLinks += 2;
    }

    debug_print_yellow("Utilization on Network links:");
    for(auto it = syndbSim.topo->networkLinkVector.begin(); it != syndbSim.topo->networkLinkVector.end(); it++){
        
        auto map = (*it)->byte_count;
        auto it_byte_count = map.begin();

        switch_id_t sw1 = it_byte_count->first;
        byte_count_t byteCount1 = it_byte_count->second;
        it_byte_count++;
        switch_id_t sw2 = it_byte_count->first;
        byte_count_t byteCount2 = it_byte_count->second;

        util1 = (double)(byteCount1 * 8) / syndbSim.totalTime;
        util2 = (double)(byteCount2 * 8) / syndbSim.totalTime;

        percent_util1 = (util1 / syndbConfig.networkLinkSpeedGbps) * 100.0;
        percent_util2 = (util2 / syndbConfig.networkLinkSpeedGbps) * 100.0;

        debug_print("Link ID {}: towards sw{}: {} | towards sw{}: {}", (*it)->id, sw1, util1, sw2, util2);

        networkLinksPercentUtilSum += percent_util1;
        networkLinksPercentUtilSum += percent_util2;
        numNetworkLinks += 2;
    }

    ndebug_print_yellow("#####  Network load summary  #####");
    ndebug_print("ToR Links: {}", torLinksPercentUtilSum / numTorLinks);
    ndebug_print("Network Links: {}", networkLinksPercentUtilSum / numNetworkLinks);

}


void Simulation::cleanUp(){
    
    // Why this is needed? When std::list is destroyed, if its members are pointers, only the pointers are destroyed, not the objects pointed by the pointers.
    // BUT for shared_ptrs, when they are deleted, they do destruct the member objects.
    // SO this function is actually NOT really required.

    // Clean-up the members of the syndbSim

    // Clean-up the topo
}
