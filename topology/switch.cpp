#include <stdexcept>
#include <fmt/core.h>
#include "topology/switch.hpp"
#include "utils/logger.hpp"
#include "simulation/simulation.hpp"
#include "simulation/config.hpp"


Switch::Switch(switch_id_t id){
    this->id = id;
    this->hop_delay = syndbConfig.switchHopDelayNs;
}



status_t Switch::routeNormalPkt(normalpkt_p pkt, routeInfo &rinfo){
    /* 
        Logic: look for pkt's dest host in neighbor hosts. If yes, get the link.
               Else, compute dst_tor_id 
     */
    switch_id_t dstTorId;
    
    auto it = this->neighborHostTable.find(pkt->dstHost);
    
    if(it != neighborHostTable.end()){ // DstHost is in the neighbor hosts
        log_debug(fmt::format("[Switch: {}, Pkt: {}] DstHost {} is the next hop!", this->id, pkt->id, pkt->dstHost));
        
        // Indicate that next dst is a host
        rinfo.nextHopId.host_id = pkt->dstHost;
        rinfo.nextHopType = NextNodeType::HostNode;

        // Set the next link
        rinfo.nextLink = it->second; 

        return SUCCESS;
    }
    else{ // DstHost is not directly reachable

        // Get the dst ToR ID based on the topology's logic
        dstTorId = syndbSim.topo.getTorId(pkt->dstHost);

        status_t s = this->routeToDstSwitch(dstTorId, rinfo); 

        if(s == FAILURE){
            std::string errMsg = fmt::format("Switch {} found no route for pkt {} to dstHost {}", this->id, pkt->id, pkt->dstHost);
            throw std::logic_error(errMsg);
        }

        return s;
    }
    
    

}


status_t Switch::routeToDstSwitch(switch_id_t dstSwitchId, routeInfo &rinfo){
    
    // First, find if the dstSwitchId is in the neighbor switch list

    auto it = this->neighborSwitchTable.find(dstSwitchId);

    if(it != this->neighborSwitchTable.end()){ // dst switch is a neighbor
        
        rinfo.nextHopType = SwitchNode;
        rinfo.nextHopId.switch_id = dstSwitchId;
        rinfo.nextLink = it->second;

        return SUCCESS;
    }

    // Not a neighbor. Now refer to the routing table.
    
    switch_id_t nextHopSwitchId;
    auto it1 = this->routingTable.find(dstSwitchId);

        if (it1 != routingTable.end()){ // Route found

            nextHopSwitchId = it1->second;
            // Indicate that next dst is a switch
            rinfo.nextHopId.switch_id = nextHopSwitchId;
            rinfo.nextHopType = NextNodeType::SwitchNode;

            // Find the link to the switch
            auto it2 = this->neighborSwitchTable.find(nextHopSwitchId);

            if(it2 == neighborSwitchTable.end()){ // No link to next hop
                // This should NOT happen
                std::string errMsg = fmt::format("Switch {} has no link to next hop switch {} for dst Tor {}", this->id, nextHopSwitchId, dstSwitchId);
                throw std::logic_error(errMsg);
            }

            rinfo.nextLink = it2->second;

            return SUCCESS;

        }
        else // Err No known route to dst ToR
        {
            return FAILURE;
        }
}

