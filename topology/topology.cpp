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

    link_p newLink = createNewToRLink();

    // Update things on Host
    host->torLink = newLink;
    host->torSwitch = tor;

    // Update things on Switch
    neighbor_switch_table_pair newNeighborHost(host->id, newLink);
    tor->neighborHostTable.insert(newNeighborHost);

    // Update things on Topology
    host_tor_map_pair newHost(host->id, tor->id);
    this->hostTorMap.insert(newHost);

}

void Topology::connectSwitchToSwitch(switch_p s1, switch_p s2){

    link_p newLink = createNewNetworLink();

    // Update things on s1
    neighbor_switch_table_pair newNeighborSwitch2(s2->id, newLink);
    s1->neighborSwitchTable.insert(newNeighborSwitch2);

    // Update things on s2
    neighbor_switch_table_pair newNeighborSwitch1(s1->id, newLink);
    s2->neighborSwitchTable.insert(newNeighborSwitch1);

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
    routing_table_pair newRoute_s0(s1->id, s2->id);
    s0->routingTable.insert(newRoute_s0);

    // Setup routing on s1
    routing_table_pair newRoute_s1(s0->id, s2->id);
    s1->routingTable.insert(newRoute_s1);

    // Setup routing on s2
    // NO need since both the other ToRs are neighbors
}

link_p Topology::createNewToRLink(){

    link_p newLink = link_p(new Link(this->getNextLinkId(), syndbConfig.torLinkSpeedGbps));
    this->torLinkVector.push_back(newLink);

    return newLink;
}

link_p Topology::createNewNetworLink(){

    link_p newLink = link_p(new Link(this->getNextLinkId(), syndbConfig.networkLinkSpeedGbps));
    this->networkLinkVector.push_back(newLink);

    return newLink;
}

switch_p Topology::createNewSwitch(){
    switch_p newSwitch = switch_p(new Switch(this->getNextSwitchId()));

    this->switchIDMap.insert(switch_id_map_pair(newSwitch->id, newSwitch));

    return newSwitch;  
}

host_p Topology::createNewHost(){
    host_p newHost = host_p(new Host(this->getNextHostId()));

    this->hostIDMap.insert(host_id_map_pair(newHost->id, newHost));

    return newHost;
}
