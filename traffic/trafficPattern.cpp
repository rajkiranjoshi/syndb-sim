#include <chrono>
#include <thread>
#include <limits>
#include "traffic/trafficPattern.hpp"
#include "simulation/simulation.hpp"
#include "topology/host.hpp"
#include "utils/logger.hpp"

host_id_t SimpleTopoTrafficPattern::applyTrafficPattern(){
    // Hard-coded logic for destination host. Mainly for SimpleTopology
    return (this->parentHostId + 1) % 2; // 1 for 0 and 0 for 1
}

AlltoAllTrafficPattern::AlltoAllTrafficPattern(host_id_t hostId): TrafficPattern::TrafficPattern(hostId){
    this->nextDst = (hostId + 1) % syndbConfig.numHosts;
}

host_id_t AlltoAllTrafficPattern::applyTrafficPattern(){
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

host_id_t FtUniformTrafficPattern::applyTrafficPattern(){
    return this->fixedDstHost;
}

FtMixedTrafficPattern::FtMixedTrafficPattern(host_id_t hostId):TrafficPattern::TrafficPattern(hostId){
    
    uint64_t seed1 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    this->randTrafficType = std::default_random_engine(seed1);

    // sleep to ensure suffiently different next seed
    std::this_thread::sleep_for(std::chrono::microseconds(1)); 

    uint64_t seed2 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    this->randIntraRackHost = std::default_random_engine(seed2);

    // sleep to ensure suffiently different next seed
    std::this_thread::sleep_for(std::chrono::microseconds(1)); 

    uint64_t seed3 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    this->randInterRackHost = std::default_random_engine(seed3);

}

void FtMixedTrafficPattern::initTopoInfo(){
    
    host_id_t min = std::numeric_limits<host_id_t>::max();
    host_id_t max = 0;
    host_id_t rackHostId;
    
    switch_id_t torId = syndbSim.topo->getTorId(this->parentHostId);
    const Switch* tor = syndbSim.topo->getSwitchById(torId);

    for(auto it = tor->neighborHostTable.begin(); it != tor->neighborHostTable.end(); it++){
        rackHostId = it->first;

        if(rackHostId < min)
            min = rackHostId;
        
        if(rackHostId > max)
            max = rackHostId;
    }

    this->rackMinId = min;
    this->rackMaxId = max;

}

TrafficDstType FtMixedTrafficPattern::getDstType(){
    
    TrafficDstType distType;
    
    uint_fast32_t randval = this->randTrafficType() % 100;
    
    if(randval <= syndbConfig.ftMixedPatternPercentIntraRack)
        distType = TrafficDstType::IntraRack;
    else
        distType = TrafficDstType::InterRack;
    
    return distType;
}

host_id_t FtMixedTrafficPattern::getRandomIntraRackHost(){
    
    host_id_t dstHost;
    uint_fast32_t randval;

    host_id_t numHostsPerRack = syndbConfig.fatTreeTopoK / 2;

    do {
        randval = this->randIntraRackHost() % numHostsPerRack;
        dstHost = this->rackMinId + randval;
    } while (dstHost == this->parentHostId);

    return dstHost;

}

host_id_t FtMixedTrafficPattern::getRandomInterRackHost(){
    
    host_id_t dstHost;
    uint_fast32_t randval;

    do
    {
        dstHost = this->randInterRackHost() % syndbConfig.numHosts;
    } while (dstHost >= this->rackMinId && dstHost <= this->rackMaxId);

    return dstHost;
}

host_id_t FtMixedTrafficPattern::applyTrafficPattern(){

    TrafficDstType dstType = this->getDstType(); 
    
    if(dstType == TrafficDstType::IntraRack){
        return this->getRandomIntraRackHost();
    }
    else{
        return this->getRandomInterRackHost();
    }
}
