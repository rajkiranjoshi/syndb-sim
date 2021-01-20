#include <stdexcept>
#include <fmt/core.h>
#include "topology/switch.hpp"
#include "utils/logger.hpp"
#include "utils/utils.hpp"
#include "simulation/simulation.hpp"
#include "simulation/config.hpp"


Switch::Switch(switch_id_t id){
    this->id = id;
    this->hop_delay = syndbConfig.switchHopDelayNs;
}


syndb_status_t Switch::routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo){
    // Two steps for updating the rsinfo struct
    // Step 1: Do the routing: determines nextSwitch, nextLink, nextLink's queue (correct next_idle_time)
    // Step 2: (common) Do the scheduling: determines the pktNextForwardTime for the correct nextLink's queue

    switch_id_t dstTorId;

    dstTorId = syndbSim.topo.getTorId(pkt->dstHost);
    
    if(dstTorId == this->id){ // intra-rack routing
        // Figure-out the link + queue and call schedulePkt() --> updates the q_next_idle_time
        // nextSwitch is NULL (since host)
        // Update the rsinfo struct

        host_tor_link_p hostTorLink;

        auto search = this->neighborHostTable.find(pkt->dstHost);
        if(search != this->neighborHostTable.end()){
            
            hostTorLink = search->second;
            this->schedulePkt(pkt->size, pktArrivalTime, hostTorLink->speed, hostTorLink->next_idle_time_to_host);

            rsinfo.nextSwitch = NULL; // next hop is a host
            rsinfo.pktNextForwardTime = hostTorLink->next_idle_time_to_host;

            return syndb_status_t::success;
        }
        else
        { // this should never happen
            std::string msg = fmt::format("ToR switch {} doesn't have entry for neighbor host {} in neighborHostTable", this->id, pkt->dstHost);
            throw std::logic_error(msg);
        }
    } // end of intra-rack routing case
    else // inter-rack routing
    {
        // call the switch-to-switch routing+scheduling since we know the dstTorId
        return this->routeScheduleToDstSwitch(pkt->size, pktArrivalTime, dstTorId, rsinfo, PacketType::NormalPkt);
        
    } // end of inter-rack routing

}

syndb_status_t Switch::routeScheduleTriggerPkt(triggerpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo){
    switch_p nextHopSwitch;
    network_link_p nextLink;

    return this->routeScheduleToDstSwitch(pkt->size, pktArrivalTime, pkt->dstSwitchId, rsinfo, PacketType::TriggerPkt);

}


syndb_status_t Switch::routeScheduleToDstSwitch(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const switch_id_t dstSwitchId, routeScheduleInfo &rsinfo, PacketType ptype){
    
    switch_p nextHopSwitch;
    network_link_p nextLink;
    
    nextHopSwitch = this->getNextHop(dstSwitchId);

    auto search = this->neighborSwitchTable.find(nextHopSwitch->id);
    if(search != this->neighborSwitchTable.end()){
        nextLink = search->second;
        
        // Choose different queues on the nextLink based on the packet type
        if(ptype == PacketType::NormalPkt){
            // Schedule on the normal queue
            schedulePkt(pktsize, pktArrivalTime, nextLink->speed, nextLink->next_idle_time[nextHopSwitch->id]);
            rsinfo.pktNextForwardTime = nextLink->next_idle_time[nextHopSwitch->id];
        }
        else if (ptype == PacketType::TriggerPkt){
            // Schedule on the priority queue
            schedulePkt(pktsize, pktArrivalTime, nextLink->speed, nextLink->next_idle_time_priority[nextHopSwitch->id]);
            rsinfo.pktNextForwardTime = nextLink->next_idle_time_priority[nextHopSwitch->id];
        }

        rsinfo.nextSwitch = nextHopSwitch;
        

        return syndb_status_t::success;
    }
    else
    { // this should never happen
        std::string msg = fmt::format("Switch {} has no link to next hop switch {} for dst switch {}", this->id, nextHopSwitch->id, dstSwitchId);
        throw std::logic_error(msg);
    }
        
}


switch_p Switch::getNextHop(switch_id_t dstSwitchId){
    
    // First, find if the dstSwitchId is in the neighbor switch list

    auto it = this->neighborSwitchTable.find(dstSwitchId);

    if(it != this->neighborSwitchTable.end()){ // dst switch is a neighbor
        // So it itself is the nextHop
        // simply return the shared_pointer to the dstSwitchId 
        return syndbSim.topo.getSwitchById(dstSwitchId); 
    }

    // Not a neighbor. Now refer to the routing table.
    
    switch_id_t nextHopSwitchId;
    auto it1 = this->routingTable.find(dstSwitchId);

    if (it1 != routingTable.end()){ // Next hop found

        nextHopSwitchId = it1->second;
        
        // Simply return the pointer to the nextHopSwitch
        return syndbSim.topo.getSwitchById(nextHopSwitchId);

    }
    else // Err No known route to dst ToR
    {   // ideally this should never happen
        std::string msg = fmt::format("Switch {} couldn't find any route to dst switch {}", this->id, dstSwitchId); 
        throw std::logic_error(msg); 
    }
}

void Switch::schedulePkt(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const link_speed_gbps_t linkSpeed, sim_time_t &qNextIdleTime){
    
    sim_time_t pktSendTime, timeAfterSwitchHop, pktNextSerializeStartTime;

    // *earliest* time when we can start serialization on the link
    timeAfterSwitchHop = pktArrivalTime + this->hop_delay;

    // *actual* time when we can start serialization assuming FIFO queuing on the next link
    pktNextSerializeStartTime = std::max<sim_time_t>(timeAfterSwitchHop, qNextIdleTime);

    // Time when serialization would end and pkt can be forwarded to next hop
    pktSendTime = pktNextSerializeStartTime + getSerializationDelay(pktsize, linkSpeed);

    // Schedule the packet on the link
    qNextIdleTime = pktSendTime;
}


void Switch::receiveNormalPkt(normalpkt_p pkt, sim_time_t rxTime){
    this->ringBuffer.insertPrecord(pkt->id, rxTime); 
}


void Switch::receiveTriggerPkt(triggerpkt_p pkt, sim_time_t rxTime){
    
    switch_id_t dstSwitchId;

    if(pkt->dstSwitchId != this->id){
        std::string msg = "Multi-hop trigger pkt received. This should NEVER happen!";
        throw std::logic_error(msg);
    }

    /* This has to be the destination. All trigger pkts are single-hop */
    // Check if this trigger pkt has been received in the past
    bool triggerSeenBefore = this->triggerHistory.find(pkt->triggerId) != this->triggerHistory.end();
    
    if(triggerSeenBefore){ // already broadcast forwarded before
        return; // do nothing
    }

    // For logging latency of triggerPkts
    #ifdef DEBUG
    
    auto it1 = syndbSim.TriggerPktLatencyMap.find(pkt->triggerId);
    if(it1 != syndbSim.TriggerPktLatencyMap.end()){ // already exists
        it1->second.rxSwitchTimes[this->id] = rxTime;
    }
    else
    {
        // Create a new info entry
        triggerPktLatencyInfo info;
        info.triggerOrigTime = pkt->triggerTime;
        info.rxSwitchTimes[this->id] = rxTime;

        // Insert it in the global map
        syndbSim.TriggerPktLatencyMap[pkt->triggerId] = info;
    }
    

    #endif

    // Broadcast forward to neighbors with source pruning
    switch_id_t srcSwitch = pkt->srcSwitchId;
    

    auto it2 = this->neighborSwitchTable.begin();
    for(it2; it2 != this->neighborSwitchTable.end(); it2++){
        
        if(it2->first == srcSwitch)
            continue; // source pruning

        dstSwitchId = it2->first;

        this->createSendTriggerPkt(dstSwitchId, pkt->triggerId, pkt->triggerOriginSwId, pkt->triggerTime);

    }

// TODO: add code for TriggerPktLatency logging
// OLD code 
/* 
    if(pkt->dstSwitchId == this->id){ // This is the dst Switch
        #ifdef DEBUG
        syndbSim.TriggerPktLatencyMap[pkt->id].end_time = rxTime;
        #endif
    }
*/

}

void Switch::generateTrigger(){
    trigger_id_t newTriggerId;
    syndb_status_t status;
    switch_id_t dstSwitchId;
    routeScheduleInfo rsinfo;

    newTriggerId = syndbSim.getNextTriggerPktId();

    this->triggerHistory.insert(newTriggerId); // this switch has already seen this triggerPkt

    // Iterate over the neighbors and schedule a triggerPkt for each neighbor
    auto it = this->neighborSwitchTable.begin();

    for(it; it != this->neighborSwitchTable.end(); it++){

        dstSwitchId = it->first;

        this->createSendTriggerPkt(dstSwitchId, newTriggerId, this->id, syndbSim.currTime);

        /* 
        latencyRecord.src = srcSwitch->id; 
        latencyRecord.dst = newTriggerPkt->dstSwitchId;
        latencyRecord.start_time = syndbSim.currTime;
        latencyRecord.end_time = 0;

        syndbSim.TriggerPktLatencyMap[newTriggerPkt->id] = latencyRecord;
         */
    }
}

void Switch::createSendTriggerPkt(switch_id_t dstSwitchId, trigger_id_t triggerId, switch_id_t originSwitchId, sim_time_t origTriggerTime){

    syndb_status_t status;
    routeScheduleInfo rsinfo;

    triggerpkt_p newTriggerPkt = triggerpkt_p(new TriggerPkt(triggerId, syndbConfig.triggerPktSize)); 
    newTriggerPkt->srcSwitchId = this->id;
    newTriggerPkt->dstSwitchId = dstSwitchId; // neighbor switch ID
    newTriggerPkt->triggerOriginSwId = originSwitchId;
    newTriggerPkt->triggerTime = origTriggerTime; 

    status = this->routeScheduleTriggerPkt(newTriggerPkt, syndbSim.currTime, rsinfo);
    if(status == syndb_status_t::success)
        debug_print_yellow("Routed and scheduled Trigger Pkt {} from switch {} --> {}", newTriggerPkt->triggerId, this->id, dstSwitchId);
    else
    {
        std::string msg = "Failed to routeScheduleTriggerPkt";
        throw std::logic_error(msg); 
    }
    

    // Create, fill and add a new trigger pkt event
    pktevent_p<triggerpkt_p> newEvent = pktevent_p<triggerpkt_p>(new PktEvent<triggerpkt_p>);

    newEvent->pkt = newTriggerPkt;
    newEvent->pktForwardTime = rsinfo.pktNextForwardTime;
    newEvent->currSwitch = switch_p(this); 
    newEvent->nextSwitch = rsinfo.nextSwitch;

    syndbSim.TriggerPktEventList.push_back(newEvent);
}

