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

#ifdef DEBUG
    // Just for debugging if pktGen is disabled
    if(pktInfo.size == 0){ // pktGen has not generated any pkt
        // Set nextPkt as NULL
        this->nextPkt = NULL;
        this->nextPktTime = syndbSim.currTime;

        return;
    }
#endif

    this->nextPkt = pktInfo.pkt;
    
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
    
    routeScheduleInfo rsinfo;

    // Step 1: Pass the pkt to ToR for its own processing
    this->torSwitch->receiveNormalPkt(this->nextPkt); // can parallelize switch's processing?

    // Step 2: Call the routing on the ToR switch to get rsinfo
    syndb_status_t s = this->torSwitch->routeScheduleNormalPkt(this->nextPkt, this->nextPktTime, rsinfo); 

    if(s != syndb_status_t::success){
        std::string msg = fmt::format("Host {} failed to send Pkt to host {} since routing on the ToR failed!", this->id, this->nextPkt->dstHost);
        throw std::logic_error(msg);
    }

    // Create, fill and add a new normal pkt event
    pktevent_p<normalpkt_p> newPktEvent = pktevent_p<normalpkt_p>(new PktEvent<normalpkt_p>());
    newPktEvent->pkt = this->nextPkt;
    newPktEvent->pktForwardTime = rsinfo.pktNextForwardTime;
    newPktEvent->currSwitch = this->torSwitch;
    newPktEvent->nextSwitch = rsinfo.nextSwitch;

    syndbSim.NormalPktEventList.push_front(newPktEvent);

    // Generate next pkt to send from the host
    this->generateNextPkt();

}