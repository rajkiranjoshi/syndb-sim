#ifndef EVENT_H
#define EVENT_H

#include "traffic/packet.hpp"
#include "topology/switch.hpp"

typedef struct NormalPktEvent
{
    normalpkt_p pkt;
    sim_time_t pktForwardTime;
    switch_p currSwitch;
    switch_p nextSwitch;
} NormalPktEvent;

typedef std::shared_ptr<NormalPktEvent> normalpktevent_p;

typedef struct TriggerPktEvent
{
    triggerpkt_p pkt;
    sim_time_t pktForwardTime;
    switch_p currSwitch;
    switch_p nextSwitch;
} TriggerPktEvent;

typedef std::shared_ptr<TriggerPktEvent> triggerpktevent_p;

#endif