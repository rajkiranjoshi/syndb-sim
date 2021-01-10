#ifndef TRAFFICGEN_H
#define TRAFFICGEN_H

#include "utils/types.hpp"
#include "traffic/packet.hpp"


typedef struct nextPacketInfo
{
    pkt_size_t size;
    time_t delay;

    inline nextPacketInfo(pkt_size_t size, time_t delay); // constructor

} nextPacketInfo;

/* trafficGenerator struct */
typedef struct 
{
    link_speed_gbps_t torLinkSpeed;
    load_t load;

    nextPacketInfo getNextPacketInfo();

} trafficGenerator;


#endif
