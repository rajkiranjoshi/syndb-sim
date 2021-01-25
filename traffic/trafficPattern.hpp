#ifndef TRAFFICPATTERN_H
#define TRAFFICPATTERN_H

#include "utils/types.hpp"

struct TrafficPattern
{
    virtual switch_id_t applyTrafficPattern(switch_id_t srcSwitchId) = 0;
};

struct SimpleTopoTrafficPattern : TrafficPattern
{
    switch_id_t applyTrafficPattern(switch_id_t srcSwitchId);
};



#endif
