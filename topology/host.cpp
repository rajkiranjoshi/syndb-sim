#include <algorithm>
#include "topology/host.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"

Host::Host(){
    
    this->torLink = NULL;
    this->torSwitch = NULL;

    this->nextPkt = NULL;
    this->nextPktTime = 0;

}

Host::Host(host_id_t id):Host::Host(){
    this->id = id;

    this->trafficGen = trafficGenerator(syndbConfig.torLinkSpeedGbps, syndbConfig.hostTrafficGenLoadPercent, id);
}

void Host::init(){
    packetInfo pktInfo = this->trafficGen.getNextPacket();

    this->nextPkt = pktInfo.pkt;

    // sim_time_t nextPktSerializeStart = std::max(syndbSim.currTime + pktInfo.sendDelay, this->torLink->next_idle_time);

}