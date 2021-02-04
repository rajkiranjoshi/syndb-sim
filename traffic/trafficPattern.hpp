#ifndef TRAFFICPATTERN_H
#define TRAFFICPATTERN_H

#include <random>
#include "utils/types.hpp"

struct TrafficPattern
{
    host_id_t parentHostId;

    TrafficPattern(host_id_t hostId) {this->parentHostId = hostId;}; 
    virtual host_id_t applyTrafficPattern() = 0;
};

struct SimpleTopoTrafficPattern : TrafficPattern
{
    // Using constructor same as the base class.
    using TrafficPattern::TrafficPattern; 
    host_id_t applyTrafficPattern();
};

struct AlltoAllTrafficPattern : TrafficPattern
{
    host_id_t nextDst;
    
    AlltoAllTrafficPattern(host_id_t hostId);
    host_id_t applyTrafficPattern();
};

struct FtUniformTrafficPattern : TrafficPattern
{
    host_id_t fixedDstHost;

    FtUniformTrafficPattern(host_id_t hostId); 
    host_id_t applyTrafficPattern();
};

struct FtMixedTrafficPattern : TrafficPattern
{
    host_id_t rackMinId, rackMaxId;

    std::default_random_engine randTrafficType;
    std::default_random_engine randIntraRackHost;
    std::default_random_engine randInterRackHost;

    FtMixedTrafficPattern(host_id_t hostId);
    host_id_t applyTrafficPattern();

    void initTopoInfo();

    private:
    TrafficDstType getDstType();
    host_id_t getRandomIntraRackHost();
    host_id_t getRandomInterRackHost();
};



#endif
