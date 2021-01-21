
#include <cmath>
#include "traffic/trafficGenerator.hpp"


nextPacketInfo::nextPacketInfo(pkt_size_t s, time_t d){
    size = s;
    delay = d;
}

/* Simple pkt generator for now: continuous generation  */
/* Load variation: return pkt_size 0, if no packet is to be sent. */
nextPacketInfo trafficGenerator::getNextPacketInfo(){
    pkt_size_t size = 1500; // in bytes
    pkt_size_t size_on_wire = size + 24;
    sim_time_t delay = 0;
    sim_time_t flowdelay = 0;
    //((size_on_wire * 8) / this->torLinkSpeed);


    size = myRandomFromCDF.getNextPacketSize(); 
    flowdelay = myRandomFromCDF.getNextFlowDelay();
    

    nextPacketInfo pktInfo(size, delay);
    return pktInfo;
}

int trafficGenerator::loadTrafficDistribution (string packetsizeDistFile, string flowarrivalDistFile) {

    myRandomFromCDF.loadCDFs("traffic-dist/fb_webserver_packetsizedist_cdf.csv", "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv");
}
