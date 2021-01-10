#include <stdexcept>
#include <fmt/core.h>
#include "topology/switch.hpp"
#include "utils/logger.hpp"
#include "config.hpp"

status_t Switch::routeNormalPkt(normalpkt_p pkt, routeInfo &rinfo){
    /* 
        Logic: look for pkt's dest host in neighbor hosts. If yes, get the link.
               Else, compute dst_tor_id 
     */
    switch_id_t dstTorId, nextHopSwitchId;
    
    auto it = this->NeighborHostTable.find(pkt->dstHost);
    
    if(it != NeighborHostTable.end()){ // DstHost is in the neighbor hosts
        log_debug(fmt::format("[Switch: {}, Pkt: {}] DstHost {} is the next hop!", this->id, pkt->id, pkt->dstHost));
        
        // Indicate that next dst is a host
        rinfo.nextHopId.host_id = pkt->dstHost;
        rinfo.nextHopType = NextNodeType::HostNode;

        // Set the next link
        rinfo.nextLink = it->second; 
    }
    else{ // DstHost is not directly reachable

        // Get the dst ToR ID based on the topology's logic
        dstTorId = topo.getTorId(pkt->dstHost);

        auto it1 = this->RoutingTable.find(dstTorId);

        if (it1 != RoutingTable.end()){ // Route found

            nextHopSwitchId = it1->second;
            // Indicate that next dst is a switch
            rinfo.nextHopId.switch_id = nextHopSwitchId;
            rinfo.nextHopType = NextNodeType::SwitchNode;

            // Find the link to the switch
            auto it2 = this->NeighborSwitchTable.find(nextHopSwitchId);

            if(it2 == NeighborSwitchTable.end()){ // No link to next hop
                // This should NOT happen
                std::string errMsg = fmt::format("Switch {} has no link to next hop switch {} for dst Tor {}", this->id, nextHopSwitchId, dstTorId);
                throw std::logic_error(errMsg);
            }

            rinfo.nextLink = it2->second;

        }
        else // Err No known route to dst ToR
        {
            std::string errMsg = fmt::format("Switch {} found no route for pkt {} to dstHost {}", this->id, pkt->id, pkt->dstHost);
            throw std::logic_error(errMsg);
        }
        
    }
    
    return status_t::Success;

}

