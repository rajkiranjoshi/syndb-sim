#include "traffic/trafficGenerator.hpp"
#include "simulation/simulation.hpp"
#include "utils/utils.hpp"


packetInfo::packetInfo(normalpkt_p pkt, pkt_size_t size, time_t sendDelay, time_t serializeDelay){
    this->pkt = pkt;
    this->size = size;
    this->sendDelay = sendDelay;
    this->serializeDelay = serializeDelay;
}

TrafficGenerator::TrafficGenerator(link_speed_gbps_t linkSpeed, load_t load, switch_id_t hostId){
    this->torLinkSpeed = linkSpeed;
    this->load = load;
    this->parentHostId = hostId;
}


/* Simple pkt generator for now: continuous generation  */
/* Load variation: return pkt_size 0, if no packet is to be sent. */
packetInfo SimpleTrafficGenerator::getNextPacket(){
    
    pkt_size_t size = 1500;
    
    sim_time_t serializeDelay = getSerializationDelay(size, this->torLinkSpeed);
    sim_time_t sendDelay = 0;

    pkt_id_t pktId = syndbSim.getNextPktId();
    normalpkt_p pkt = normalpkt_p(new NormalPkt(pktId, size));
    
    pkt->srcHost = this->parentHostId;

    // Hard-coded logic for destination host. Mainly for SimpleTopology
    pkt->dstHost = (this->parentHostId + 1) % 2; // 1 for 0 and 0 for 1

    return packetInfo(pkt, size, sendDelay, serializeDelay);
}
