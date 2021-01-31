
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

TrafficGenerator::TrafficGenerator(link_speed_gbps_t linkSpeed, load_t load, switch_id_t hostId){
    this->torLinkSpeed = linkSpeed;
    this->load = load;
    this->parentHostId = hostId;
}


/* Data-ceter Traffic generator based on   */
/* Load variation: return pkt_size 0, if no packet is to be sent. */

packetInfo DcTrafficGenerator::getNextPacket(){
   
    pkt_size_t pkt_size = myRandomFromCDF.getNextPacketSize(); 
    sim_time_t sendDelay = myRandomFromCDF.getNextPacketDelay();
    

    sim_time_t serializeDelay = getSerializationDelay(pkt_size, this->torLinkSpeed);

    pkt_id_t pktId = syndbSim.getNextPktId();
    normalpkt_p pkt = normalpkt_p(new NormalPkt(pktId, pkt_size));
    
    pkt->srcHost = this->parentHostId;

    return packetInfo(pkt, pkt_size, sendDelay, serializeDelay);
}

int DcTrafficGenerator::loadTrafficDistribution (std::string packetsizeDistFile, std::string flowarrivalDistFile) {
    pkt_size_t base_size = 80; // in bytes
    pkt_size_t size_on_wire = base_size + 24;
    // int pps = ((this->torLinkSpeed * 1000000000)/ (size_on_wire * 8));
    // sim_time_t min_delay_ns = ((double)1/(double)pps * 1000000000);
    sim_time_t min_delay_ns = (size_on_wire * 8) / this->torLinkSpeed;


    myRandomFromCDF.loadCDFs("traffic-dist/fb_webserver_packetsizedist_cdf.csv", "traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv", min_delay_ns);
    return 0;
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
       

    return packetInfo(pkt, size, sendDelay, serializeDelay);
}

int SimpleTrafficGenerator::loadTrafficDistribution (std::string packetsizeDistFile, std::string flowarrivalDistFile) {

    return 0;
}

// int main () {
//     RandomFromCDF myRandomFromCDF;
//     myRandomFromCDF.loadCDFs("../traffic-dist/fb_webserver_packetsizedist_cdf.csv", "../traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv");
//     for (int i=0;i<10;i++) {
//         cout<<myRandomFromCDF.getNextPacketSize()<<"\n";
//     }

//     for (int i=0;i<10;i++) {
//         cout<<myRandomFromCDF.getNextFlowDelay()<<"\n";
//     }
//     return 0;
// }