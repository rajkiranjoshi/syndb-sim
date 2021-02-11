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

Switch* Topology::getSwitchById(switch_id_t id){
    auto it = this->switchIDMap.find(id);
    if(it != this->switchIDMap.end()){ // found the switch_p
        return it->second.get();
    }
    else
    {
        std::string msg = fmt::format("No switch_p found for switch_id {}. This should NEVER happen!", id);
        throw std::logic_error(msg); 
    }
}

Host* Topology::getHostById(host_id_t hostId){
    auto it = this->hostIDMap.find(hostId);
    if(it != this->hostIDMap.end()){ // found the switch_p
        return it->second.get();
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

host_tor_link_p Topology::createNewToRLink(){

    // host_tor_link_p newLink = host_tor_link_p(new HostTorLink(this->getNextLinkId(), syndbConfig.torLinkSpeedGbps));
    host_tor_link_p newLink = std::make_shared<HostTorLink>(this->getNextLinkId(), syndbConfig.torLinkSpeedGbps);
    this->torLinkVector.push_back(newLink);

    return newLink;
}

network_link_p Topology::createNewNetworLink(switch_id_t sw1, switch_id_t sw2){

    // network_link_p newLink = network_link_p(new NetworkLink(this->getNextLinkId(), syndbConfig.networkLinkSpeedGbps, sw1, sw2));
    network_link_p newLink = std::make_shared<NetworkLink>(this->getNextLinkId(), syndbConfig.networkLinkSpeedGbps, sw1, sw2);
    this->networkLinkVector.push_back(newLink);

    return newLink;
}

switch_p Topology::createNewSwitch(SwitchType type){
    switch_p newSwitch;
    
    switch (type)
    {
        case SwitchType::FtTor:
            newSwitch = std::make_shared<SwitchFtTor>(this->getNextSwitchId());
            break;
        case SwitchType::FtAggr:
            newSwitch = std::make_shared<SwitchFtAggr>(this->getNextSwitchId());
            break;
        case SwitchType::FtCore:
            newSwitch = std::make_shared<SwitchFtCore>(this->getNextSwitchId()); 
            break;
        case SwitchType::Simple:
        default:
            newSwitch = std::make_shared<SimpleSwitch>(this->getNextSwitchId());
            break;  
    }

    newSwitch->type = type;

    // newSwitch->swPktArrivalFile = std::unique_ptr<std::ofstream>(new std::ofstream(fmt::format("./pktarrival/{}_sw{}.txt", switchTypeToString(type), newSwitch->id), std::ofstream::out)); 
     
    this->switchIDMap[newSwitch->id] = newSwitch; 
    this->switchTypeIDMap[type].insert(newSwitch->id); 

    return newSwitch;  
}

host_p Topology::createNewHost(bool trafficGenDisabled){

    // host_p newHost = host_p(new Host(this->getNextHostId(), trafficGenDisabled));
    host_p newHost = std::make_shared<Host>(this->getNextHostId(), trafficGenDisabled);

    this->hostIDMap[newHost->id] = newHost; 

    return newHost;
}


void Topology::addHostToTor(host_p &host, switch_p &tor){

    host_tor_link_p newLink = createNewToRLink();

    // Update things on Host
    host->torLink = newLink;
    host->torSwitch = tor.get();

    // Update things on Switch
    tor->neighborHostTable[host->id] = newLink.get();

    // Update things on Topology
    this->hostTorMap[host->id] = tor->id;

    // debug_print("Connected h{} to sw{}", host->id, tor->id);

}

void Topology::connectSwitchToSwitch(switch_p &s1, switch_p &s2){

    network_link_p newLink = createNewNetworLink(s1->id, s2->id);

    // Update things on s1
    s1->neighborSwitchTable[s2->id] = newLink.get();

    // Update things on s2
    s2->neighborSwitchTable[s1->id] = newLink.get();

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

/* 
    h0 --- s0 --- s1 --- s2 --- s3 --- s4 --- h1
*/
void LineTopology::buildTopo(){
    
    host_p h0, h1;
    switch_p s0, s1, s2, s3, s4;

    // Create all hosts
    h0 = this->createNewHost();
    h1 = this->createNewHost(true);

    // Create all switches
    s0 = this->createNewSwitch(SwitchType::Simple); 
    s1 = this->createNewSwitch(SwitchType::Simple); 
    s2 = this->createNewSwitch(SwitchType::Simple);
    s3 = this->createNewSwitch(SwitchType::Simple); 
    s4 = this->createNewSwitch(SwitchType::Simple);

    this->addHostToTor(h0, s0);
    this->addHostToTor(h1, s4);

    this->connectSwitchToSwitch(s0, s1);
    this->connectSwitchToSwitch(s1, s2);
    this->connectSwitchToSwitch(s2, s3);
    this->connectSwitchToSwitch(s3, s4);

    // Setup routing on the switches. Need to downcast shared_pointer to SimpleSwitch
    auto s0_ss = std::dynamic_pointer_cast<SimpleSwitch>(s0);
    auto s1_ss = std::dynamic_pointer_cast<SimpleSwitch>(s1);
    auto s2_ss = std::dynamic_pointer_cast<SimpleSwitch>(s2);
    auto s3_ss = std::dynamic_pointer_cast<SimpleSwitch>(s3);
    auto s4_ss = std::dynamic_pointer_cast<SimpleSwitch>(s4);

    // Add routes for ToR switches when not direct neighbor
    s0_ss->routingTable[s4->id] = s1->id;
    s1_ss->routingTable[s4->id] = s2->id;
    s2_ss->routingTable[s0->id] = s1->id;
    s2_ss->routingTable[s4->id] = s3->id;
    s3_ss->routingTable[s0->id] = s2->id;
    s4_ss->routingTable[s0->id] = s3->id;

    ndebug_print_yellow("Built Line topo:");
    ndebug_print("h0 --- s0 --- s1 --- s2 --- s3 --- s4 --- h1\n");

}
