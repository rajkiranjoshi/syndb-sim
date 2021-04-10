#ifndef DATAPARSER_H
#define DATAPARSER_H

#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include "utils/types.hpp"

#define WINDOW_SIZE 1000 * 1000

#define PREFIX_STRING_FOR_DATA_FILES "dump"
#define PREFIX_FILE_PATH "/home/user/syndb"


struct PacketInfo {
    pkt_id_t id;
    host_id_t srcHost;
    host_id_t dstHost;

    sim_time_t switchIngressTime;
};

struct TriggerInfo {
    int triggerId;

    switch_id_t originSwitch;
    sim_time_t triggerTime;
    std::unordered_map<switch_id_t, sim_time_t> mapOfSwitchTriggerTime;
};


struct DataParser {

    /* data */
    std::string prefixStringForFileName;

    std::fstream triggerFilePointer, sourceDestinationFilePointer;
    std::vector<std::fstream> switchFilePointers;

    std::vector<TriggerInfo> listOfTriggers;

    ~DataParser();
    DataParser() = default;
    DataParser(std::string prefixFilePath, std::string prefixStringForFileName, switch_id_t numberOfSwitches);

    std::string executeShellCommand(const char* command);
    void getTriggerInfo(switch_id_t numberOfSwitches);
    std::map<pkt_id_t, PacketInfo> getWindowForSwitch(switch_id_t switchID, sim_time_t triggerTime, pkt_id_t windowSize, bool isTriggerSwitch);
    float getCorrelationBetweenPrecordWindows(std::map<pkt_id_t, PacketInfo> precordWindowForTriggerSwitch, std::map<pkt_id_t, PacketInfo> precordWindowForCurrentSwitch);
};



#endif
