#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "utils/types.hpp"

typedef struct Config 
{
    const sim_time_t timeIncrementNs = 100;
    const sim_time_t totalTimeMSecs = 1;

    const std::string topo = "SimpleTopology";

    const link_speed_gbps_t torLinkSpeedGbps = 100;
    const link_speed_gbps_t networkLinkSpeedGbps = 100;

    const load_t hostTrafficGenLoadPercent = 100;

    const sim_time_t switchHopDelayNs = 1000;

} Config;

extern Config syndbConfig;

#endif