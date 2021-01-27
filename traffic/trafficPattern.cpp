#include "traffic/trafficPattern.hpp"
#include "simulation/simulation.hpp"
#include "topology/host.hpp"
#include "utils/logger.hpp"

switch_id_t SimpleTopoTrafficPattern::applyTrafficPattern(){
    // Hard-coded logic for destination host. Mainly for SimpleTopology
    return (this->parentHostId + 1) % 2; // 1 for 0 and 0 for 1
}

AlltoAllTrafficPattern::AlltoAllTrafficPattern(host_id_t hostId): TrafficPattern::TrafficPattern(hostId){
    this->nextDst = (hostId + 1) % syndbConfig.numHosts;
}

switch_id_t AlltoAllTrafficPattern::applyTrafficPattern(){
    host_id_t dstHost;

    dstHost = this->nextDst;

    this->nextDst = (this->nextDst + 1) % syndbConfig.numHosts;

    return dstHost; 
    /* Termination Logic:
       - when dstHost becomes same as parentHostId, it's a loopback pkt
       - after a loopback pkt is detected in Host::sendpkt(), no more calls to generatePkt() and so no more calls to applyTrafficPattern()
    */
}


FtUniformTrafficPattern::FtUniformTrafficPattern(host_id_t hostId):TrafficPattern::TrafficPattern(hostId){
    host_id_t halfPoint = syndbConfig.numHosts / 2;

    if(hostId < halfPoint){ // this host is in the left half
        this->fixedDstHost = hostId + halfPoint;
    }
    else // this host is in the right half
    {
        this->fixedDstHost = hostId - halfPoint;
    }
}

switch_id_t FtUniformTrafficPattern::applyTrafficPattern(){
    return this->fixedDstHost;
}