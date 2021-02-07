#include <stdexcept>
#include <fmt/core.h>
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "topology/topology.hpp"
#include "utils/logger.hpp"


Topology::Topology(){
    this->switchTypeIDMap[SwitchType::Simple] = std::set<switch_id_t>();
    this->switchTypeIDMap[SwitchType::FtTor] = std::set<switch_id_t>();
    this->switchTypeIDMap[SwitchType::FtAggr] = std::set<switch_id_t>();
    this->switchTypeIDMap[SwitchType::FtCore] = std::set<switch_id_t>();
}

SwitchType Topology::getSwitchTypeById(switch_id_t id){
    auto typeit = this->switchTypeIDMap.begin();
    for(typeit; typeit != this->switchTypeIDMap.end(); typeit++){
        if(typeit->second.find(id) != typeit->second.end()) // Found the switch ID
            return typeit->first; 
    }

    std::string msg = fmt::format("Switch id {} has no assigned type. This should NEVER happen!",id);
    throw std::logic_error(msg);
}

switch_p Topology::getSwitchById(switch_id_t id){
    auto it = this->switchIDMap.find(id);
    if(it != this->switchIDMap.end()){ // found the switch_p
        return it->second;
    }
    else
    {
        std::string msg = fmt::format("No switch_p found for switch_id {}. This should NEVER happen!", id);
        throw std::logic_error(msg); 
    }
}

host_p Topology::getHostById(host_id_t hostId){
    auto it = this->hostIDMap.find(hostId);
    if(it != this->hostIDMap.end()){ // found the switch_p
        return it->second;
    }
    else
    {
        std::string msg = fmt::format("No host_p found for host_id {}. This should NEVER happen!", hostId);
        throw std::logic_error(msg); 
    }
}

switch_id_t Topology::getTorId(host_id_t hostId){

    auto it = hostTorMap.find(hostId);

    if(it == hostTorMap.end()){
        std::string msg = fmt::format("No ToR found for host {} in the topology!", hostId);
        throw std::logic_error(msg);
    }
    // We have found the ToR, if we reach here
    return it->second;
}


void Topology::addHostToTor(host_p host, switch_p tor){

    host_tor_link_p newLink = createNewToRLink();

    // Update things on Host
    host->torLink = newLink;
    host->torSwitch = tor;

    // Update things on Switch
    tor->neighborHostTable[host->id] = newLink;

    // Update things on Topology
    this->hostTorMap[host->id] = tor->id;

    // debug_print("Connected h{} to sw{}", host->id, tor->id);

}

void Topology::connectSwitchToSwitch(switch_p s1, switch_p s2){

    network_link_p newLink = createNewNetworLink(s1->id, s2->id);

    // Update things on s1
    s1->neighborSwitchTable[s2->id] = newLink;

    // Update things on s2
    s2->neighborSwitchTable[s1->id] = newLink;

    // debug_print("Connected sw{} to sw{}", s1->id, s2->id);

}


void SimpleTopology::buildTopo(){

    host_id_t hostId;
    switch_id_t switchId;
    host_p h0, h1, h2, h3;
    switch_p s0, s1, s2;

    // Create all hosts
    h0 = this->createNewHost();
    h1 = this->createNewHost();
    // h2 = this->createNewHost();
    // h3 = this->createNewHost();
    
    // Create all switches
    s0 = this->createNewSwitch(SwitchType::Simple); 
    s1 = this->createNewSwitch(SwitchType::Simple); 
    s2 = this->createNewSwitch(SwitchType::Simple); 

    this->addHostToTor(h0, s0);
    // this->addHostToTor(h2, s0);
    this->addHostToTor(h1, s1);
    // this->addHostToTor(h3, s1);

    connectSwitchToSwitch(s0, s2);
    connectSwitchToSwitch(s1, s2);

    // Setup routing on s0. Need to downcast shared_pointer to SimpleSwitch
    auto s0_ss = std::dynamic_pointer_cast<SimpleSwitch>(s0); 
    s0_ss->routingTable[s1->id] = s2->id;

    // Setup routing on s1. Need to downcast shared_pointer to SimpleSwitch
    auto s1_ss = std::dynamic_pointer_cast<SimpleSwitch>(s1);
    s1_ss->routingTable[s0->id] = s2->id;

    // Setup routing on s2
    // NO need since both the other ToRs are neighbors
}

host_tor_link_p Topology::createNewToRLink(){

    host_tor_link_p newLink = host_tor_link_p(new HostTorLink(this->getNextLinkId(), syndbConfig.torLinkSpeedGbps));
    this->torLinkVector.push_back(newLink);

    return newLink;
}

network_link_p Topology::createNewNetworLink(switch_id_t sw1, switch_id_t sw2){

    network_link_p newLink = network_link_p(new NetworkLink(this->getNextLinkId(), syndbConfig.networkLinkSpeedGbps, sw1, sw2));
    this->networkLinkVector.push_back(newLink);

    return newLink;
}

switch_p Topology::createNewSwitch(SwitchType type){
    switch_p newSwitch;
    
    switch (type)
    {
        case SwitchType::FtTor:
            newSwitch = switch_p(new SwitchFtTor(this->getNextSwitchId()));
            break;
        case SwitchType::FtAggr:
            newSwitch = switch_p(new SwitchFtAggr(this->getNextSwitchId()));
            break;
        case SwitchType::FtCore:
            newSwitch = switch_p(new SwitchFtCore(this->getNextSwitchId())); 
            break;
        case SwitchType::Simple:
        default:
            newSwitch = switch_p(new SimpleSwitch(this->getNextSwitchId()));
            break;  
    }

    newSwitch->type = type;
     
    this->switchIDMap[newSwitch->id] = newSwitch; 
    this->switchTypeIDMap[type].insert(newSwitch->id); 

    return newSwitch;  
}

host_p Topology::createNewHost(bool trafficGenDisabled){
    host_p newHost = host_p(new Host(this->getNextHostId(), trafficGenDisabled));

    this->hostIDMap[newHost->id] = newHost; 

    return newHost;
}
