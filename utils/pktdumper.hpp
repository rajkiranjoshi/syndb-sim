 #ifndef PACKETDUMPER_H
#define PACKETDUMPER_H

#include <map>
#include <vector>
#include <ctime>
#include <fstream>
#include <time.h>

#include "traffic/packet.hpp"
#include "traffic/incastGenerator.hpp"
#include "traffic/triggerGenerator.hpp"
#include "topology/switch.hpp"
#include <fmt/core.h>
#include <fmt/color.h>


// need to declare logger.hpp first because of conflict in fmt
#include <spdlog/async.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>

#define QUEUE_SIZE 32768 * 2
    typedef struct PktDumper
{
    std::string prefixStringForFileName;

    std::shared_ptr<spdlog::logger> triggerFilePointer, sourceDestinationFilePointer, incastFilePointer, simSummaryFilePointer;
    std::vector<std::shared_ptr<spdlog::logger>> switchFilePointers;

    ~PktDumper();
    PktDumper() = default;
    
    void openFiles(switch_id_t numberOfSwitches, host_id_t numberOfHosts);
    void dumpPacket(const normalpkt_p pkt);
    void dumpTriggerInfo(const trigger_id_t &triggerId, triggerInfo &tinfo, const SwitchType &switchType);
    void dumpIncastInfo(const incastScheduleInfo &incastInfo);
    void logSimSummary(const std::string &msg);

} PktDumper;



#endif

