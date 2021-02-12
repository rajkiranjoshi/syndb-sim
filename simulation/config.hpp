#ifndef PROFILING
#define PROFILING 0
#endif

#if PROFILING 
#include "simulation/config_profiling.hpp"
#else

#ifndef CONFIG_H
#define CONFIG_H

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
    // const TopologyType topoType = TopologyType::Simple;
    // static const switch_id_t numSwitches = 3;
    // static const uint numHosts = 4;
    // const TrafficPatternType trafficPatternType = TrafficPatternType::SimpleTopo;
    // const sim_time_t triggerInitialDelay = 1500000; // 1.5ms for Line topo 10G runs
    // const link_speed_gbps_t torLinkSpeedGbps = 100;
    // const link_speed_gbps_t networkLinkSpeedGbps = 100;

    /* LineTopo 10ms Expt Params */
    /* Do NOT comment out */ const pkt_size_t fixedPktSizeForSimpleTrafficGen = 101; //1500;
    // const float totalTimeMSecs = 10;
    // const TopologyType topoType = TopologyType::Line;
    // static const switch_id_t numSwitches = 5;
    // static const uint numHosts = 2;
    // const TrafficPatternType trafficPatternType = TrafficPatternType::SimpleTopo;
    // const sim_time_t triggerInitialDelay = 1500000; // 1.5ms for Line topo 10ms run
    // static const uint numTriggersPerSwitchType = 100;
    // const link_speed_gbps_t torLinkSpeedGbps = 10;
    // const link_speed_gbps_t networkLinkSpeedGbps = 10;
    // const TrafficGenType trafficGenType = TrafficGenType::Continuous;
    
    
    /* FatTree Topo 100ms Expt Params */
    /* Do NOT comment out */ static const ft_scale_t fatTreeTopoK = 24; // Fat Tree scale k
    /* Do NOT comment out */ const uint8_t ftMixedPatternPercentIntraRack = 75;
    /* Do NOT comment out */ const uint8_t targetBaseNetworkLoadPercent = 40;  /* 30 -> 25%, 40 -> 31%, 50 -> 36% */
    const float totalTimeMSecs = 100;
    const TopologyType topoType = TopologyType::FatTree;
    static const host_id_t numHosts = (fatTreeTopoK * fatTreeTopoK * fatTreeTopoK)/4;
    static const switch_id_t numSwitches = (fatTreeTopoK * fatTreeTopoK) + ((fatTreeTopoK * fatTreeTopoK)/4);   
    const TrafficPatternType trafficPatternType = TrafficPatternType::FtMixed;
    const TrafficGenType trafficGenType = TrafficGenType::Distribution;
    const sim_time_t triggerInitialDelay = 15000000; // 15ms for k=24 fatTree topo 100ms run
    static const uint numTriggersPerSwitchType = 100;
    const link_speed_gbps_t torLinkSpeedGbps = 100;
    const link_speed_gbps_t networkLinkSpeedGbps = 100;
    

    // const TrafficPatternType trafficPatternType = TrafficPatternType::AlltoAll;
    // const TrafficPatternType trafficPatternType = TrafficPatternType::FtUniform;


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

    // Fat Tree specific config options
    static const int numCoreSwitches = (fatTreeTopoK/2) * (fatTreeTopoK/2);
  
    const std::string packetSizeDistFile = "traffic-dist/fb_webserver_packetsizedist_cdf.csv";

    const std::string flowArrivalDistFile = "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv";

    void readConfigFile(std::string configFileName);
    void setValues(std::string key, long value);
    // void setValues(std::string key, std::string value);

    
} Config;

extern Config syndbConfig;

#endif

#endif
