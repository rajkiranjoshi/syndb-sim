#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "utils/types.hpp"
// #include "topology/topology.hpp"

typedef struct Config 
{
    const sim_time_t timeIncrementNs = 100;
    const float totalTimeMSecs = 0.1;

    const TopologyType topoType = TopologyType::Simple;
    // const TopologyType topoType = TopologyType::FatTree;
    
    static const ft_scale_t fatTreeTopoK = 6; // Fat Tree scale k

    // const TrafficGenType trafficGenType = TrafficGenType::Distribution;
    const TrafficGenType trafficGenType = TrafficGenType::Simple;

    const link_speed_gbps_t torLinkSpeedGbps = 100;
    const link_speed_gbps_t networkLinkSpeedGbps = 100;

    const load_t hostTrafficGenLoadPercent = 100;

    const sim_time_t switchHopDelayNs = 1000;

    // SyNDB specific config options
    static const uint32_t ringBufferSize = 100; // large size for simulation "oracle"
    static const pkt_size_t triggerPktSize = 60;

    // Fat Tree specific config options
    static const uint numCoreSwitches = (fatTreeTopoK/2) * (fatTreeTopoK/2);

} Config;

extern Config syndbConfig;

#endif