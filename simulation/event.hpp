#ifndef EVENT_H
#define EVENT_H

#include "traffic/packet.hpp"
#include "topology/switch.hpp"

template<typename T>
struct PktEvent
{
    T pkt; // could be normalpkt_p or triggerpkt_p
    sim_time_t pktForwardTime;
    switch_p currSwitch;
    switch_p nextSwitch; // NULL if next hop is dstHost
    
    PktEvent();
    ~PktEvent();
};


// Following is called an "alias template". 
// It is used for typedef equivalent of template structs/classes
template<typename T>
using pktevent_p = std::shared_ptr<PktEvent<T>>;



#endif