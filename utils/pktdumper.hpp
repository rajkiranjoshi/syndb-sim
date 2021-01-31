#ifndef PACKETDUMPER_H
#define PACKETDUMPER_H

#include <map>
#include "traffic/packet.hpp"
#include "topology/switch.hpp"

struct triggerInfo {
    sim_time_t triggerOrigTime;
    switch_id_t originSwitch;
    std::map<switch_id_t, sim_time_t> rxSwitchTimes;
};

typedef struct PktDumper
{
    /* data */

    void dumpPacket(normalpkt_p pkt);
    void dumpTriggerInfo(trigger_id_t triggerId, triggerInfo tinfo, SwitchType switchType);
} PktDumper;



#endif

