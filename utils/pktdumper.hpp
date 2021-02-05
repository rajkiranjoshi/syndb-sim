#ifndef PACKETDUMPER_H
#define PACKETDUMPER_H

#include <map>
#include <vector>
#include <ctime>
#include <fstream>
#include <time.h>
#include "traffic/packet.hpp"
#include "topology/switch.hpp"

struct triggerInfo {
    sim_time_t triggerOrigTime;
    switch_id_t originSwitch;
    std::map<switch_id_t, sim_time_t> rxSwitchTimes;
};

typedef struct PktDumper
{
    std::string prefixStringForFileName;

    std::fstream triggerFilePointer;
    std::vector<std::fstream> switchFilePointers;
    std::vector<std::fstream> hostFilePointers;

    ~PktDumper();
    PktDumper() = default;
    
    void openFiles(switch_id_t numberOfSwitches, host_id_t numberOfHosts);
    void dumpPacket(normalpkt_p pkt);
    void dumpTriggerInfo(trigger_id_t triggerId, triggerInfo tinfo, SwitchType switchType);
} PktDumper;



#endif

