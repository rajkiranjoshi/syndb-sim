#ifndef TRAFFICGEN_H
#define TRAFFICGEN_H

#include "utils/types.hpp"
#include "traffic/packet.hpp"


typedef struct
{
    pkt_size_t size;
    time_t delay;
} nextPacketInfo;

/* trafficGenerator struct */
typedef struct 
{
    link_speed_gbps_t torLinkSpeed;
    load_t load;

    nextPacketInfo getNextPacketInfo();

} trafficGenerator;


#endif
