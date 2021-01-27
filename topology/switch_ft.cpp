#include <fmt/core.h>
#include "simulation/simulation.hpp"
#include "topology/switch_ft.hpp"


switch_p SwitchFtTorAggr::getNextHop(host_id_t dstHostId){
    switch_id_t nextHopSwitchId;
    racklocal_host_id_t rackLocalHostId;
    switch_p nextHopSwitch;

    rackLocalHostId = dstHostId % (this->fatTreeScaleK / 2);

    auto it = this->routingTable.find(rackLocalHostId);

    if(it == this->routingTable.end()){ // this should NOT happen
        std::string msg = fmt::format("rackLocalHostId {} not found in Tor/Aggr switch {} routing table", rackLocalHostId, this->id);

        throw std::logic_error(msg); 
    }

    nextHopSwitchId = it->second; 
    nextHopSwitch = syndbSim.topo->getSwitchById(nextHopSwitchId);

    return nextHopSwitch;
}

syndb_status_t SwitchFtTor::routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo){

    switch_id_t dstTorId;

    dstTorId = syndbSim.topo->getTorId(pkt->dstHost);
    
    if(dstTorId == this->id){ // intra-rack routing
        return this->intraRackRouteNormalPkt(pkt, pktArrivalTime, rsinfo); 
    } // end of intra-rack routing case
    else // inter-rack routing
    {
        switch_p nextHopSwitch;
        nextHopSwitch = this->getNextHop(pkt->dstHost);

        // call the switch-to-switch scheduling since we know the nextHopSwitch
        return this->scheduleToNextHopSwitch(pkt->size, pktArrivalTime, nextHopSwitch, rsinfo, PacketType::NormalPkt);

    }
}


syndb_status_t SwitchFtAggr::routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo){
    switch_id_t dstTorId;
    switch_p nextHopSwitch;
    dstTorId = syndbSim.topo->getTorId(pkt->dstHost);

    // find if dstTor is in the neighbors (if intra-pod routing)
    auto it = this->neighborSwitchTable.find(dstTorId);
    if(it != this->neighborSwitchTable.end()){ // intra-pod routing
        nextHopSwitch = syndbSim.topo->getSwitchById(dstTorId);
    }
    else // inter-pod routing. Send to the uplink core
    {
        // Get the uplink core switch from the routing table
        nextHopSwitch = this->getNextHop(pkt->dstHost);
    }

    // call the switch-to-switch scheduling since we know the nextHopSwitch
    return this->scheduleToNextHopSwitch(pkt->size, pktArrivalTime, nextHopSwitch, rsinfo, PacketType::NormalPkt);

}


switch_p SwitchFtCore::getNextHop(host_id_t dstHostId){
    switch_p nextHopSwitch;
    pod_id_t podId;
    ft_scale_t kBy2 = this->fatTreeScaleK / 2;
    
    podId = dstHostId / (kBy2 * kBy2);

    auto it = this->routingTable.find(podId);

    if(it == this->routingTable.end()){
        std::string msg = fmt::format("No route to pod {} on core switch {}", podId, this->id);
        throw std::logic_error(msg);
    }
    
    // it->second is the aggr switch id
    nextHopSwitch = syndbSim.topo->getSwitchById(it->second);

    return nextHopSwitch;   

}

syndb_status_t SwitchFtCore::routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo){

    switch_p nextHopSwitch;
    nextHopSwitch = this->getNextHop(pkt->dstHost);

    // call the switch-to-switch scheduling since we know the nextHopSwitch
    return this->scheduleToNextHopSwitch(pkt->size, pktArrivalTime, nextHopSwitch, rsinfo, PacketType::NormalPkt);

}
