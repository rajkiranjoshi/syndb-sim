#include <string>
#include <cassert>
#include "utils/types.hpp"

#define LOGGING 1
#define HOP_DELAY_NOISE 1
#define RING_BUFFER 0
#define TRIGGERS_ENABLED 1
#define INCASTS_ENABLED 0

typedef struct Config 
{
    static const sim_time_t timeIncrementNs = 100;

    // IMPORTANT: update numHosts and numSwitches as per the topology
    
    /* LineTopo 10ms Expt Params */
    const pkt_size_t fixedPktSizeForSimpleTrafficGen = 101; //1500;
    const float totalTimeMSecs = 10;
    const TopologyType topoType = TopologyType::Line;
    static const switch_id_t numSwitches = 5;
    static const uint numHosts = 2;
    const TrafficPatternType trafficPatternType = TrafficPatternType::SimpleTopo;
    const sim_time_t triggerInitialDelay = 1500000; // 1.5ms for Line topo 10ms run with 10k ring buffer
    static const uint numTriggersPerSwitchType = 100;
    const link_speed_gbps_t torLinkSpeedGbps = 10;
    const link_speed_gbps_t networkLinkSpeedGbps = 10;
    const TrafficGenType trafficGenType = TrafficGenType::Continuous;
    
    /* Incast Related Params */
    const uint8_t percentIncastTime = 10;
    const host_id_t incastFanInRatio = numHosts / 4; // 25% of the total hosts
    const host_id_t percentTargetIncastHosts = 30;

    // const load_t hostTrafficGenLoadPercent = 100;

    const sim_time_t switchHopDelayNs = 500;
    const sim_time_t minSwitchHopDelayNs = 450;
    const sim_time_t maxSwitchHopDelayNs = 550;

    // SyNDB specific config options
    static const pkt_id_t ringBufferSize = 10000; // 10K
    static const pkt_size_t triggerPktSize = 60;

    const std::string packetSizeDistFile = "traffic-dist/fb_webserver_packetsizedist_cdf.csv";
    const std::string flowArrivalDistFile = "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv";

    /* Other params NOT used, but needed for compilation. */
    // FatTree Topo 100ms Expt Params
    static const ft_scale_t fatTreeTopoK = 24; // Fat Tree scale k
    const uint8_t ftMixedPatternPercentIntraRack = 75;
    const uint8_t targetBaseNetworkLoadPercent = 40;  /* 30 -> 25%, 40 -> 31%, 50 -> 36% */
    static const int numCoreSwitches = (fatTreeTopoK/2) * (fatTreeTopoK/2);

} Config;

extern Config syndbConfig;

