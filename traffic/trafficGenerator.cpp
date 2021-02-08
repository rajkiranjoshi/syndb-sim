
#include <cmath>
#include "traffic/trafficGenerator.hpp"
#include "simulation/simulation.hpp"
#include "utils/utils.hpp"






TrafficGenerator::~TrafficGenerator(){
    // this->interPktGapFile.close();
}

/* Data-ceter Traffic generator based on   */
/* Load variation: return pkt_size 0, if no packet is to be sent. */

void DcTrafficGenerator::getNextPacket(packetInfo &pktInfo){
   
    pktInfo.size = myRandomFromCDF.getNextPacketSize(); 
    pktInfo.sendDelay = myRandomFromCDF.getNextPacketDelay();
    
    // this->interPktGapFile << sendDelay << std::endl;

}

int DcTrafficGenerator::loadTrafficDistribution (std::string packetsizeDistFile, std::string flowarrivalDistFile) {
    pkt_size_t base_size = 80; // in bytes
    pkt_size_t size_on_wire = base_size + 24;
    // int pps = ((this->torLinkSpeed * 1000000000)/ (size_on_wire * 8));
    // sim_time_t min_delay_ns = ((double)1/(double)pps * 1000000000);
    sim_time_t min_delay_ns = (size_on_wire * 8) / syndbConfig.torLinkSpeedGbps;


    myRandomFromCDF.loadCDFs("traffic-dist/fb_webserver_packetsizedist_cdf.csv", "traffic-dist/packetinterarrival_ns_cdf.csv");
    return 0;
}

/* Simple pkt generator for now: continuous generation  */
/* Load variation: return pkt_size 0, if no packet is to be sent. */
void SimpleTrafficGenerator::getNextPacket(packetInfo &pktInfo){
    
    pktInfo.size = syndbConfig.fixedPktSizeForSimpleTrafficGen;
    pktInfo.sendDelay = 0; 

}

int SimpleTrafficGenerator::loadTrafficDistribution (std::string packetsizeDistFile, std::string flowarrivalDistFile) {

    return 0;
}

// int main () {
//     TrafficGenerator *myGen = new  DcTrafficGenerator(100, 100, 1);
//     myGen->loadTrafficDistribution("../traffic-dist/fb_webserver_packetsizedist_cdf.csv", "../traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv");
//     for (int i=0;i<100;i++) {
//         packetInfo pktinfo = myGen->getNextPacket();
//         printf("Pkt Size = %d, sendDelay=%ld\n", pktinfo.size, pktinfo.sendDelay);
//     }
//     return 0;
// }