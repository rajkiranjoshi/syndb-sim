
#include <cmath>
#include "traffic/trafficGenerator.hpp"
#include "simulation/simulation.hpp"
#include "utils/utils.hpp"


packetInfo::packetInfo(normalpkt_p pkt, pkt_size_t size, time_t sendDelay, time_t serializeDelay){
    this->pkt = pkt;
    this->size = size;
    this->sendDelay = sendDelay;
    this->serializeDelay = serializeDelay;
}

trafficGenerator::trafficGenerator(link_speed_gbps_t linkSpeed, load_t load, switch_id_t hostId){
    this->torLinkSpeed = linkSpeed;
    this->load = load;
    this->parentHostId = hostId;
}


/* Simple pkt generator for now: continuous generation  */
/* Load variation: return pkt_size 0, if no packet is to be sent. */
packetInfo trafficGenerator::getNextPacket(){
    pkt_size_t base_size = 80; // in bytes
    pkt_size_t size = 1500;
    pkt_size_t size_on_wire = base_size + 24;
    sim_time_t delay = 0;
    sim_time_t flowdelay = 0;
    //considering a packet size of 80-byte


    int pkt_size = myRandomFromCDF.getNextPacketSize(); 
    int pkt_delay_ns = myRandomFromCDF.getNextPacketDelay();
    

    sim_time_t serializeDelay = getSerializationDelay(size, this->torLinkSpeed);
    sim_time_t sendDelay = 0;

    pkt_id_t pktId = syndbSim.getNextPktId();
    normalpkt_p pkt = normalpkt_p(new NormalPkt(pktId, size));
    
    pkt->srcHost = this->parentHostId;

    // Figure out how much delay is needed based on the load


    // Hard-coded logic for destination host
    pkt->dstHost = (this->parentHostId + 1) % 2; // 1 for 0 and 0 for 1

    return packetInfo(pkt, size, sendDelay, serializeDelay);
}

int trafficGenerator::loadTrafficDistribution (string packetsizeDistFile, string flowarrivalDistFile) {
    pkt_size_t base_size = 80; // in bytes
    pkt_size_t size_on_wire = base_size + 24;
    int pps = ((size_on_wire * 8) / this->torLinkSpeed);
    sim_time_t min_delay_ns = (1/pps * 1000000000);


    myRandomFromCDF.loadCDFs("traffic-dist/fb_webserver_packetsizedist_cdf.csv", "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv", min_delay_ns);
}
