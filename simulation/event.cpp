#include <fmt/core.h>
#include "simulation/event.hpp"
#include "simulation/simulation.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"

template<typename T>
PktEvent<T>::PktEvent(){
    debug_print(fmt::format("New empty PktEvent constructed!"));
}

template<typename T>
PktEvent<T>::~PktEvent(){
    debug_print(fmt::format("Event with packet id {} destructed!", this->pkt->id));
}

/*  The logic for routing pkts between switches only */
template<typename T>
void PktEvent<T>::doForwarding(const routeScheduleInfo &rinfo){ 
    
    network_link_p pktNextLink;
    switch_p pktNextSwitch;
    sim_time_t pktNextSendTime, timeAfterSwitchHop, pktNextSerializeStartTime;

    
    pktNextLink = rinfo.nextLink;
    pktNextSwitch = syndbSim.topo.getSwitchById(rinfo.nextHopId.switch_id);

    // time when we can start serialization *earliest* on the next switch
    timeAfterSwitchHop = this->pktForwardTime + this->nextSwitch->hop_delay;

    // actual time when we can start serialization assuming FIFO queuing on the next link
    pktNextSerializeStartTime = std::max<sim_time_t>(timeAfterSwitchHop, pktNextLink->next_idle_time[rinfo.nextHopId.switch_id]);

    // Time when serialization would end and pkt can be forwarded to next hop
    pktNextSendTime = pktNextSerializeStartTime + getSerializationDelay(this->pkt->size, pktNextLink->speed);

    // Schedule the packet on the link
    pktNextLink->next_idle_time[rinfo.nextHopId.switch_id] = pktNextSendTime;

    this->currSwitch = this->nextSwitch;
    this->nextSwitch = pktNextSwitch;
    this->pktForwardTime = pktNextSendTime;

}

/* 
    To avoid linking error for the template's methods,
    instantiating the template class with the two possible types
*/

template class PktEvent<normalpkt_p>;
template class PktEvent<triggerpkt_p>;
