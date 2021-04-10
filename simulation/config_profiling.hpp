#include <string>
#include <cassert>
#include "utils/types.hpp"

#define LOGGING 0
#define HOP_DELAY_NOISE 0
#define RING_BUFFER 0
#define TRIGGERS_ENABLED 0
#define INCASTS_ENABLED 0

typedef struct Config 
{
    static const sim_time_t timeIncrementNs = 100;
    const float totalTimeMSecs = 0.5;

    // IMPORTANT: update numHosts and numSwitches as per the topology
    
    /* FatTree Topo Params */
    static const ft_scale_t fatTreeTopoK = 24; // Fat Tree scale k
    const TopologyType topoType = TopologyType::FatTree;
    static const host_id_t numHosts = (fatTreeTopoK * fatTreeTopoK * fatTreeTopoK)/4;
    static const switch_id_t numSwitches = (fatTreeTopoK * fatTreeTopoK) + ((fatTreeTopoK * fatTreeTopoK)/4);   
    const TrafficPatternType trafficPatternType = TrafficPatternType::FtUniform;
    const uint8_t ftMixedPatternPercentIntraRack = 75;
    static const uint numTriggersPerSwitchType = 15;
    const sim_time_t triggerInitialDelay = 75000; // 75us for k=24 fatTree topo 0.5ms run
    const uint8_t targetBaseNetworkLoadPercent = 40;  /* 30 -> 25%, 40 -> 31%, 50 -> 36% */
    const TrafficGenType trafficGenType = TrafficGenType::Continuous;
    const pkt_size_t fixedPktSizeForSimpleTrafficGen = 1500;
    const link_speed_gbps_t torLinkSpeedGbps = 100;
    const link_speed_gbps_t networkLinkSpeedGbps = 100;
    static const int numCoreSwitches = (fatTreeTopoK/2) * (fatTreeTopoK/2);

    /* Incast Related Params */
    const uint8_t percentIncastTime = 10;
    const host_id_t incastFanInRatio = ((fatTreeTopoK * fatTreeTopoK) * 3)/4;
    const host_id_t percentTargetIncastHosts = 30;

    const load_t hostTrafficGenLoadPercent = 100;

    const sim_time_t switchHopDelayNs = 1000;
    const sim_time_t minSwitchHopDelayNs = 950;
    const sim_time_t maxSwitchHopDelayNs = 1050;

    // SyNDB specific config options
    static const uint32_t ringBufferSize = 10; // large size for simulation "oracle"
    static const pkt_size_t triggerPktSize = 60;    
  
    const std::string packetSizeDistFile = "traffic-dist/fb_webserver_packetsizedist_cdf.csv";
    const std::string flowArrivalDistFile = "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv";
} Config;

extern Config syndbConfig;
