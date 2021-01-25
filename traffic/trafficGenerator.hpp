#ifndef TRAFFICGEN_H
#define TRAFFICGEN_H

#include "utils/types.hpp"
#include "traffic/packet.hpp"


struct packetInfo
{
    normalpkt_p pkt;
    pkt_size_t size;
    time_t sendDelay;
    time_t serializeDelay;

    packetInfo(normalpkt_p pkt, pkt_size_t size, time_t sendDelay, time_t serializeDelay); // constructor

};


struct TrafficGenerator
{
    link_speed_gbps_t torLinkSpeed;
    load_t load;
    host_id_t parentHostId;

    TrafficGenerator(link_speed_gbps_t linkSpeed, load_t load, host_id_t hostId);
    virtual packetInfo getNextPacket() = 0;
};


/* SimpleTrafficGenerator struct */
struct SimpleTrafficGenerator : TrafficGenerator
{
    // using the constructor of the base class
    using TrafficGenerator::TrafficGenerator; 
    
    /* Overriding method of the abstract class */
    packetInfo getNextPacket();

};



#endif
