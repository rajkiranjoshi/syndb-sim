#ifndef PACKETDUMPER_H
#define PACKETDUMPER_H

#include <map>
#include <vector>
#include <ctime>
#include <fstream>
#include <time.h>

#include "traffic/packet.hpp"
#include "topology/switch.hpp"
#include "utils/logger.hpp"

// need to declare logger.hpp first because of conflict in fmt
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

#define QUEUE_SIZE 32768 * 2

struct triggerInfo {
    sim_time_t triggerOrigTime;
    switch_id_t originSwitch;
    std::map<switch_id_t, sim_time_t> rxSwitchTimes;
};

typedef struct PktDumper
{
    std::string prefixStringForFileName;

    std::shared_ptr<spdlog::logger> triggerFilePointer, sourceDestinationFilePointer;
    std::vector<std::shared_ptr<spdlog::logger>> switchFilePointers;

    ~PktDumper();
    PktDumper() = default;
    
    void openFiles(switch_id_t numberOfSwitches, host_id_t numberOfHosts);
    void dumpPacket(normalpkt_p pkt);
    void dumpTriggerInfo(trigger_id_t triggerId, triggerInfo tinfo, SwitchType switchType);
} PktDumper;



#endif

