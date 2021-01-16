#include <algorithm>
#include <fmt/core.h>
#include "topology/host.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "utils/utils.hpp"

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
    sim_time_t nextPktSerializeStart = std::max<sim_time_t>(pktGenSendTime, this->torLink->next_idle_time_to_tor);
    // this->nextPktTime is the last pkt serialize end time
    this->nextPktTime = nextPktSerializeStart + pktInfo.serializeDelay;
    this->torLink->next_idle_time_to_tor = this->nextPktTime;

    // For devtest testNormalPktLatencies()
    #ifdef DEBUG
    pktTime pkt_time;
    pkt_time.srcHost = pktInfo.pkt->srcHost;
    pkt_time.dstHost = pktInfo.pkt->dstHost;
    pkt_time.start_time = pktGenSendTime; 
    pkt_time.end_time = 0; // set to zero
    syndbSim.pktLatencyMap[pktInfo.pkt->id] = pkt_time;
    #endif

}

void Host::sendPkt(){
    network_link_p pktNextLink;
    switch_p pktNextSwitch;
    sim_time_t pktNextSendTime, timeAfterSwitchHop, pktNextSerializeStartTime;
    routeScheduleInfo rinfo;

    // Step 1: Pass the pkt to ToR for its own processing
    this->torSwitch->receiveNormalPkt(this->nextPkt); // can parallelize switch's processing?

    // Step 2: Do the routing on the ToR switch. Two cases.
    // --> Case1: Intra-rack routing: dstHost is on the same rack!
    if(this->torSwitch->id == syndbSim.topo.getTorId(this->nextPkt->dstHost)){
        std::string msg = fmt::format("Host {} sending intra-rack packet to host {}. Abort!", this->id, this->nextPkt->dstHost);
        throw std::logic_error(msg);
    }
    
    // --> Case2: Inter-rack routing: dstHost is NOT on the same rack!
    // Here onward the logic is for inter-rack traffic
    this->torSwitch->routeNormalPkt(this->nextPkt, rinfo);
    pktNextLink = rinfo.nextLink;
    pktNextSwitch = syndbSim.topo.getSwitchById(rinfo.nextHopId.switch_id);
    // TODO: should this be (i) currTime OR (ii) this->nextPktTime (when the pkt was supposed to be on switch)
    // if the granularity (timeIncrementNs) is too high, using the second "might"
    timeAfterSwitchHop = this->nextPktTime + this->torSwitch->hop_delay;
    
    // Time when pkt can be serialized on the link after the ToR
    pktNextSerializeStartTime = std::max<sim_time_t>(timeAfterSwitchHop, pktNextLink->next_idle_time[rinfo.nextHopId.switch_id]);
    // Time when serialization would end and pkt can be forwarded to next hop
    pktNextSendTime = pktNextSerializeStartTime + getSerializationDelay(this->nextPkt->size, this->torLink->speed); 
    pktNextLink->next_idle_time[rinfo.nextHopId.switch_id] = pktNextSendTime; // pkt scheduled on the next link (FIFO)

    // Create, fill and add a new normal pkt event
    pktevent_p<normalpkt_p> newPktEvent = pktevent_p<normalpkt_p>(new PktEvent<normalpkt_p>());
    newPktEvent->pkt = this->nextPkt;
    newPktEvent->pktForwardTime = pktNextSendTime;
    newPktEvent->currSwitch = this->torSwitch;
    newPktEvent->nextSwitch = pktNextSwitch;

    syndbSim.NormalPktEventList.push_front(newPktEvent);

    // Generate next pkt to send from the host
    this->generateNextPkt();

}