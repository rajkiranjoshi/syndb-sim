#include <string>
#include <cassert>
#include "utils/types.hpp"

#ifndef LOGGING
#define LOGGING 1
#endif
#define HOP_DELAY_NOISE 1
#define RING_BUFFER 0
#define TRIGGERS_ENABLED 1
#define INCASTS_ENABLED 1

typedef struct Config 
{
    static const sim_time_t timeIncrementNs = 100;

    // IMPORTANT: update numHosts and numSwitches as per the topology
    
    /* SimpleTopo Params */
    const float totalTimeMSecs = 1000;
    const TopologyType topoType = TopologyType::Simple;
    static const switch_id_t numSwitches = 3;
    static const uint numHosts = 4;
    const TrafficPatternType trafficPatternType = TrafficPatternType::SimpleTopo;
    const TrafficGenType trafficGenType = TrafficGenType::Distribution;
    const sim_time_t triggerInitialDelay = 1500000; // 1.5ms for Line topo 10G runs
    static const uint numTriggersPerSwitchType = 100;
    const link_speed_gbps_t torLinkSpeedGbps = 100;
    const link_speed_gbps_t networkLinkSpeedGbps = 100;

    /* Incast Related Params */
    const uint8_t percentIncastTime = 10;
    const host_id_t incastFanInRatio = numHosts / 4; // 25% of the total hosts
    const host_id_t percentTargetIncastHosts = 30;

    // const load_t hostTrafficGenLoadPercent = 100;

    const sim_time_t switchHopDelayNs = 1000;
    const sim_time_t minSwitchHopDelayNs = 950;
    const sim_time_t maxSwitchHopDelayNs = 1050;

    // SyNDB specific config options
    static const uint32_t ringBufferSize = 10; // large size for simulation "oracle"
    static const pkt_size_t triggerPktSize = 60;

    const std::string packetSizeDistFile = "traffic-dist/fb_webserver_packetsizedist_cdf.csv";
    const std::string flowArrivalDistFile = "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv";



    /* Other params NOT used but needed for compilation */
    // LineTopo 10ms Expt Params 
    const pkt_size_t fixedPktSizeForSimpleTrafficGen = 101; //1500;  
    // FatTree Topo 100ms Expt Params 
    static const ft_scale_t fatTreeTopoK = 4; // Fat Tree scale k
    const uint8_t ftMixedPatternPercentIntraRack = 75;
    const uint8_t targetBaseNetworkLoadPercent = 40;  /* 30 -> 25%, 40 -> 31%, 50 -> 36% */
    static const int numCoreSwitches = (fatTreeTopoK/2) * (fatTreeTopoK/2);
  

} Config;

extern Config syndbConfig;

