#include <algorithm>
#include <fmt/core.h>
#include <memory>
#include "topology/host.hpp"
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"

Host::Host(host_id_t id, bool disableTrafficGen){
    this->torLink = NULL;
    this->torSwitch = NULL;
    this->nextPkt = NULL;
    this->nextPktTime = 0;
    this->prevPktTime = 0;

    this->id = id;
    this->trafficGenDisabled = disableTrafficGen;
    
    if(syndbConfig.trafficGenType == TrafficGenType::Continuous){
        this->trafficGen = std::shared_ptr<TrafficGenerator>(new SimpleTrafficGenerator());
    }
    else if (syndbConfig.trafficGenType == TrafficGenType::Distribution)
    {
        this->trafficGen = std::shared_ptr<TrafficGenerator>(new DcTrafficGenerator());
        this->trafficGen->loadTrafficDistribution(syndbConfig.packetSizeDistFile, syndbConfig.flowArrivalDistFile);
    }

    switch(syndbConfig.trafficPatternType){
        case TrafficPatternType::SimpleTopo:
            this->trafficPattern = std::shared_ptr<TrafficPattern>(new SimpleTopoTrafficPattern(this->id));
            break;
        case TrafficPatternType::AlltoAll:
            this->trafficPattern = std::shared_ptr<TrafficPattern>(new AlltoAllTrafficPattern(this->id));
            break;
        case TrafficPatternType::FtUniform:
            this->trafficPattern = std::shared_ptr<TrafficPattern>(new FtUniformTrafficPattern(this->id));
            break;
        case TrafficPatternType::FtMixed:
            this->trafficPattern = std::shared_ptr<TrafficPattern>(new FtMixedTrafficPattern(this->id));
            break;
        default:
            std::string msg = fmt::format("Host constructor failed. No way to initialize the specified traffic pattern: {}", syndbConfig.trafficPatternType);
            throw std::logic_error(msg);
            break;
    }
}

void Host::generateNextPkt(){
    

#ifdef DEBUG
    // Just for debugging if pktGen is disabled
    if(this->trafficGenDisabled == true){ // pktGen has not generated any pkt
        // Set nextPkt as NULL
        this->nextPkt = NULL;
        this->nextPktTime = syndbSim.currTime;

        return;
    }
#endif
    // Get the pktsize + delay from the trafficGen
    this->trafficGen->getNextPacket(this->nextPktInfo);

    // Construct a pkt
    this->nextPkt = std::move(syndbSim.getNewNormalPkt(syndbSim.getNextPktId(), this->nextPktInfo.size));
    this->nextPkt->srcHost = this->id;
    this->nextPkt->size = this->nextPktInfo.size;
    // Get the dstHost from the TrafficPattern
    this->nextPkt->dstHost = this->trafficPattern->applyTrafficPattern();
    this->prevPktTime = this->nextPktTime; // save curr next time to prev
    
    sim_time_t pktGenSendTime = this->nextPktTime + this->nextPktInfo.sendDelay;
    sim_time_t nextPktSerializeStart = std::max<sim_time_t>(pktGenSendTime, this->torLink->next_idle_time_to_tor);
    // this->nextPktTime is the last pkt serialize end time
    this->nextPktTime = nextPktSerializeStart + getSerializationDelay(this->nextPkt->size, syndbConfig.torLinkSpeedGbps);;
    this->torLink->next_idle_time_to_tor = this->nextPktTime;

    // Update the byte count
    this->torLink->byte_count_to_tor += this->nextPkt->size + 24; // +24 to account for on-wire PHY bits

    // Add appropriate INT data to the packet
    this->nextPkt->startTime = nextPktSerializeStart;


}

void Host::sendPkt(normalpkt_p &nextPkt, sim_time_t nextPktTime){
    
    routeScheduleInfo rsinfo;

    if(this->trafficGenDisabled)
        return;
    else if (nextPkt->dstHost == this->id){ // loopback pkt
        return;
    }
    
    // For quick testing of AlltoAll traffic pattern
    // ndebug_print("sendPkt(): {} --> {}", this->id, nextPkt->dstHost);

    // Step 1: Pass the pkt to ToR for its own processing
    this->torSwitch->receiveNormalPkt(nextPkt, nextPktTime); // can parallelize switch's processing?

    // Step 2: Call the routing + scheduling on the ToR switch to get rsinfo
    syndb_status_t s = this->torSwitch->routeScheduleNormalPkt(nextPkt, nextPktTime, rsinfo); 

    if(s != syndb_status_t::success){
        std::string msg = fmt::format("Host {} failed to send Pkt to host {} since routing on the ToR failed!", this->id, nextPkt->dstHost);
        throw std::logic_error(msg);
    }

    // Create, fill and add a new normal pkt event
    // pktevent_p<normalpkt_p> newPktEvent = pktevent_p<normalpkt_p>(new PktEvent<normalpkt_p>());
    pktevent_p<normalpkt_p> newPktEvent = std::move(syndbSim.getNewNormalPktEvent());
    newPktEvent->pkt = nextPkt;
    newPktEvent->pktForwardTime = rsinfo.pktNextForwardTime;
    newPktEvent->currSwitch = this->torSwitch;
    newPktEvent->nextSwitch = rsinfo.nextSwitch;

    syndbSim.NormalPktEventList.push_front(newPktEvent);

}
