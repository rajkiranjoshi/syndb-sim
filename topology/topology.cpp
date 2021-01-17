#include <stdexcept>
#include <fmt/core.h>
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "topology/topology.hpp"


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

}

void Topology::connectSwitchToSwitch(switch_p s1, switch_p s2){

    network_link_p newLink = createNewNetworLink(s1->id, s2->id);

    // Update things on s1
    s1->neighborSwitchTable[s2->id] = newLink;

    // Update things on s2
    s2->neighborSwitchTable[s1->id] = newLink;

}


void SimpleTopology::Topology::buildTopo(){

    host_id_t hostId;
    switch_id_t switchId;
    host_p h0, h1;
    switch_p s0, s1, s2;

    // Create all hosts
    h0 = createNewHost();
    h1 = createNewHost();
    
    // Create all switches
    s0 = createNewSwitch(); 
    s1 = createNewSwitch(); 
    s2 = createNewSwitch(); 

    addHostToTor(h0, s0);
    addHostToTor(h1, s1);

    connectSwitchToSwitch(s0, s2);
    connectSwitchToSwitch(s1, s2);

    // Setup routing on s0
    s0->routingTable[s1->id] = s2->id;

    // Setup routing on s1
    s1->routingTable[s0->id] = s2->id;

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

switch_p Topology::createNewSwitch(){
    switch_p newSwitch = switch_p(new Switch(this->getNextSwitchId()));

    this->switchIDMap[newSwitch->id] = newSwitch; 

    return newSwitch;  
}

host_p Topology::createNewHost(){
    host_p newHost = host_p(new Host(this->getNextHostId()));

    this->hostIDMap[newHost->id] = newHost; 

    return newHost;
}
