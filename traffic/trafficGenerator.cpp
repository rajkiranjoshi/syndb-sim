#include "traffic/trafficGenerator.hpp"

nextPacketInfo trafficGenerator::getNextPacketInfo(){
    pkt_size_t size = 1500;
    sim_time_t delay = ((size * 8) / this->torLinkSpeed);

    nextPacketInfo pktInfo{size, delay};

    return pktInfo;
}
