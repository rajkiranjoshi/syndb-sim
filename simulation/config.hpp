#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cassert>
#include "utils/types.hpp"

#define LOGGING 1 
#define HOP_DELAY_NOISE 1

typedef struct Config 
{
    static const sim_time_t timeIncrementNs = 100;
    const float totalTimeMSecs = 0.1;

    // IMPORTANT: update numHosts and numSwitches as per the topology
    
    /* SimpleTopo Params */
    // const TopologyType topoType = TopologyType::Simple;
    // static const switch_id_t numSwitches = 3;
    // static const uint numHosts = 4;
    // const TrafficPatternType trafficPatternType = TrafficPatternType::SimpleTopo;
    
    /* FatTree Topo Params */
    /* Do NOT comment out */ static const ft_scale_t fatTreeTopoK = 24; // Fat Tree scale k
    const TopologyType topoType = TopologyType::FatTree;
    static const host_id_t numHosts = (fatTreeTopoK * fatTreeTopoK * fatTreeTopoK)/4;
    static const switch_id_t numSwitches = (fatTreeTopoK * fatTreeTopoK) + ((fatTreeTopoK * fatTreeTopoK)/4);   
    // const TrafficPatternType trafficPatternType = TrafficPatternType::AlltoAll;
    // const TrafficPatternType trafficPatternType = TrafficPatternType::FtUniform;
    const TrafficPatternType trafficPatternType = TrafficPatternType::FtMixed;

    const uint8_t ftMixedPatternPercentIntraRack = 75;

    // const TrafficGenType trafficGenType = TrafficGenType::Distribution;
    const TrafficGenType trafficGenType = TrafficGenType::Continuous;
    const pkt_size_t fixedPktSizeForSimpleTrafficGen = 1500;

    static const uint numTriggersPerSwitchType = 5;


    const link_speed_gbps_t torLinkSpeedGbps = 100;
    const link_speed_gbps_t networkLinkSpeedGbps = 100;

    const load_t hostTrafficGenLoadPercent = 100;

    const sim_time_t switchHopDelayNs = 1000;
    const sim_time_t minSwitchHopDelayNs = 950;
    const sim_time_t maxSwitchHopDelayNs = 1050;

    // SyNDB specific config options
    static const uint32_t ringBufferSize = 10; // large size for simulation "oracle"
    static const pkt_size_t triggerPktSize = 60;

    // Fat Tree specific config options
    static const int numCoreSwitches = (fatTreeTopoK/2) * (fatTreeTopoK/2);
  
    const std::string packetSizeDistFile = "traffic-dist/fb_webserver_packetsizedist_cdf.csv";

    const std::string flowArrivalDistFile = "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv";
} Config;

extern Config syndbConfig;

#endif
