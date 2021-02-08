#ifndef EVENT_H
#define EVENT_H

#include "traffic/packet.hpp"
#include "topology/host.hpp"
#include "topology/switch.hpp"

template<typename T>
struct PktEvent
{
    T pkt; // could be normalpkt_p or triggerpkt_p
    sim_time_t pktForwardTime;
    Switch* currSwitch;
    Switch* nextSwitch; // NULL if next hop is dstHost
    
    PktEvent();
    ~PktEvent();
};


// Following is called an "alias template". 
// It is used for typedef equivalent of template structs/classes
template<typename T>
using pktevent_p = std::shared_ptr<PktEvent<T>>;

struct HostPktEvent
{
    Host* host;
    normalpkt_p pkt;

    HostPktEvent(Host* host, normalpkt_p &pkt);
};

typedef std::shared_ptr<HostPktEvent> hostpktevent_p;





#endif