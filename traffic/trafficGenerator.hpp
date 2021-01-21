#ifndef TRAFFICGEN_H
#define TRAFFICGEN_H

#include "utils/types.hpp"
#include "traffic/packet.hpp"
#include "randomGenCDF.hpp"

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
    RandomFromCDF myRandomFromCDF;
    
    nextPacketInfo getNextPacketInfo();
    int loadTrafficDistribution(string packetsizeDistFile, string flowarrivalDistFile);
} trafficGenerator;


#endif
