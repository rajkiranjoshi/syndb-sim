#ifndef DATAPARSER_H
#define DATAPARSER_H

#include <fstream>
#include <vector>
#include <map>
#include "utils/types.hpp"

struct PacketInfo {
    pkt_id_t id;
    host_id_t srcHost;
    host_id_t dstHost;

    sim_time_t switchIngressTime;
};


struct DataParser {

    /* data */
    std::string prefixStringForFileName;

    std::vector<std::fstream> switchFilePointers;
    std::vector<std::fstream> hostFilePointers;

    ~DataParser();
    DataParser() = default;
    DataParser(std::string prefixStringForFileName, switch_id_t numberOfSwitches, host_id_t numberOfHosts);

    std::map<sim_time_t, PacketInfo> getWindowForSwitch(switch_id_t switchID, sim_time_t triggerTime, pkt_id_t windowSize);
    float getCorrelationBetweenPrecordWindows(std::map<pkt_id_t, PacketInfo> precordWindowForTriggerSwitch, std::map<pkt_id_t, PacketInfo> precordWindowForCurrentSwitch);
};



#endif
