#ifndef TRAFFICGEN_H
#define TRAFFICGEN_H

#include "utils/types.hpp"
#include "traffic/packet.hpp"
#include "randomGenCDF.hpp"

typedef struct packetInfo
{
    normalpkt_p pkt;
    pkt_size_t size;
    time_t sendDelay;
    time_t serializeDelay;

    packetInfo(normalpkt_p pkt, pkt_size_t size, time_t sendDelay, time_t serializeDelay); // constructor

} packetInfo;

/* trafficGenerator struct */
typedef struct trafficGenerator
{
    link_speed_gbps_t torLinkSpeed;
    load_t load;
    RandomFromCDF myRandomFromCDF;
    
    host_id_t parentHostId;

    trafficGenerator() = default;
    trafficGenerator(link_speed_gbps_t linkSpeed, load_t load, switch_id_t hostId);
    
    packetInfo getNextPacket();
    int loadTrafficDistribution(string packetsizeDistFile, string flowarrivalDistFile);

} trafficGenerator;


#endif
