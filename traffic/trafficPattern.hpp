#ifndef TRAFFICPATTERN_H
#define TRAFFICPATTERN_H

#include "utils/types.hpp"

struct TrafficPattern
{
    host_id_t parentHostId;

    TrafficPattern(host_id_t hostId) {this->parentHostId = hostId;}; 
    virtual switch_id_t applyTrafficPattern() = 0;
};

struct SimpleTopoTrafficPattern : TrafficPattern
{
    // Using constructor same as the base class.
    using TrafficPattern::TrafficPattern; 
    switch_id_t applyTrafficPattern();
};

struct AlltoAllTrafficPattern : TrafficPattern
{
    host_id_t nextDst;
    bool finished;
    
    AlltoAllTrafficPattern(host_id_t hostId);
    switch_id_t applyTrafficPattern();
};


#endif
