
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
<<<<<<< HEAD
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
=======
packetInfo trafficGenerator::getNextPacket(){
    
    pkt_size_t size = 1500;
    
    sim_time_t serializeDelay = getSerializationDelay(size, this->torLinkSpeed);
    sim_time_t sendDelay = 0;

    pkt_id_t pktId = syndbSim.getNextPktId();
    normalpkt_p pkt = normalpkt_p(new NormalPkt(pktId, size));
    
    pkt->srcHost = this->parentHostId;

    // Hard-coded logic for destination host
    pkt->dstHost = (this->parentHostId + 1) % 2; // 1 for 0 and 0 for 1

    return packetInfo(pkt, size, sendDelay, serializeDelay);
>>>>>>> 7cbab8b6c8222fce1a3dbf6276b8dabad91ac803
}

int trafficGenerator::loadTrafficDistribution (string packetsizeDistFile, string flowarrivalDistFile) {

    myRandomFromCDF.loadCDFs("traffic-dist/fb_webserver_packetsizedist_cdf.csv", "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv");
}
