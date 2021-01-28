#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "utils/types.hpp"
// #include "topology/topology.hpp"

typedef struct Config 
{
    const sim_time_t timeIncrementNs = 100;
    const float totalTimeMSecs = 0.1;

    // IMPORTANT: update numHosts as per the topology
    // const TopologyType topoType = TopologyType::Simple;
    const TopologyType topoType = TopologyType::FatTree;
    
    static const ft_scale_t fatTreeTopoK = 4; // Fat Tree scale k

    static const int numHosts = (fatTreeTopoK * fatTreeTopoK * fatTreeTopoK)/4;
    // static const uint numHosts = 4;

    // const TrafficGenType trafficGenType = TrafficGenType::Distribution;
    const TrafficGenType trafficGenType = TrafficGenType::Continuous;

    // const TrafficPatternType trafficPatternType = TrafficPatternType::SimpleTopo;
    const TrafficPatternType trafficPatternType = TrafficPatternType::AlltoAll;
    // const TrafficPatternType trafficPatternType = TrafficPatternType::FtUniform;

    const link_speed_gbps_t torLinkSpeedGbps = 100;
    const link_speed_gbps_t networkLinkSpeedGbps = 100;

    const load_t hostTrafficGenLoadPercent = 100;

    const sim_time_t switchHopDelayNs = 1000;

    // SyNDB specific config options
    static const uint32_t ringBufferSize = 100; // large size for simulation "oracle"
    static const pkt_size_t triggerPktSize = 60;

    // Fat Tree specific config options
    static const int numCoreSwitches = (fatTreeTopoK/2) * (fatTreeTopoK/2);

    const std::string packetSizeDistFile = "traffic-dist/fb_webserver_packetsizedist_cdf.csv";

    const std::string flowArrivalDistFile = "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv";
} Config;

extern Config syndbConfig;

#endif