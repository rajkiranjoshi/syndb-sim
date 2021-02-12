#ifndef TRAFFICGEN_H
#define TRAFFICGEN_H

#include <fstream>
#include "utils/types.hpp"
#include "traffic/packet.hpp"
#include "randomGenCDF.hpp"

struct packetInfo
{
   
    pkt_size_t size;
    time_t sendDelay;
   

};


struct TrafficGenerator
{
   

    ~TrafficGenerator();
 
    virtual void getNextPacket(packetInfo &pktInfo) = 0;
    virtual int loadTrafficDistribution(std::string packetsizeDistFile, std::string packetarrivalDistFile) = 0;
};


/* SimpleTrafficGenerator struct */
struct SimpleTrafficGenerator : TrafficGenerator
{
    
    /* Overriding method of the abstract class */
    void getNextPacket(packetInfo &pktInfo);
    int loadTrafficDistribution(std::string packetsizeDistFile, std::string packetarrivalDistFile);
};

/* DC Trafficgenerator struct */
struct DcTrafficGenerator : TrafficGenerator
{
    RandomFromCDF myRandomFromCDF;
    
    // using TrafficGenerator::TrafficGenerator;
    
    void getNextPacket(packetInfo &pktInfo);
    int loadTrafficDistribution(std::string packetsizeDistFile, std::string packetarrivalDistFile);
};




#endif
