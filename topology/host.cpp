#include <algorithm>
#include <fmt/core.h>
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

void Host::generateNextPkt(){
    packetInfo pktInfo = this->trafficGen.getNextPacket();

    this->nextPkt = pktInfo.pkt;
    // TODO: Again whether (i) syndbSim.currTime OR (ii) this->nextPktTime.
    // Ideally delay is between when last pkt was sent + when next pkt should be sent
    sim_time_t pktGenSendTime = this->nextPktTime + pktInfo.sendDelay;
    sim_time_t nextPktSerializeStart = std::max<sim_time_t>(pktGenSendTime, this->torLink->next_idle_time);
    // this->nextPktTime is the last pkt serialize end time
    this->nextPktTime = nextPktSerializeStart + pktInfo.serializeDelay;
    this->torLink->next_idle_time = this->nextPktTime;
}

void Host::sendPkt(){
    link_p pktNextLink;
    switch_p pktNextSwitch;
    sim_time_t pktNextSendTime, timeAfterSwitchHop;
    routeInfo rinfo;

    this->torSwitch->routeNormalPkt(this->nextPkt, rinfo);
    if(rinfo.nextHopType == HostNode){
        std::string msg = fmt::format("Host {} sending intra-rack packet to host {}. Abort!", this->id, this->nextPkt->dstHost);
        throw std::logic_error(msg);
    }
    
    // Here onward the logic is for inter-rack traffic
    pktNextLink = rinfo.nextLink;
    pktNextSwitch = syndbSim.topo.getSwitchById(rinfo.nextHopId.switch_id);
    // TODO: should this be (i) currTime OR (ii) this->nextPktTime (when the pkt was supposed to be on switch)
    // if the granularity (timeIncrementNs) is too high, using the second "might"
    timeAfterSwitchHop = this->nextPktTime + this->torSwitch->hop_delay;
    pktNextSendTime = std::max<sim_time_t>(timeAfterSwitchHop, pktNextLink->next_idle_time);
    pktNextLink->next_idle_time = pktNextSendTime; // pkt scheduled on the next link (FIFO)

    // Create, fill and add a new normal pkt event
    normalpktevent_p newPktEvent = normalpktevent_p(new NormalPktEvent());
    newPktEvent->pkt = this->nextPkt;
    newPktEvent->pktForwardTime = pktNextSendTime;
    newPktEvent->currSwitch = this->torSwitch;
    newPktEvent->nextSwitch = pktNextSwitch;

    syndbSim.NormalPktEventList.push_front(newPktEvent);

    // Generate next pkt to send from the host
    this->generateNextPkt();

}