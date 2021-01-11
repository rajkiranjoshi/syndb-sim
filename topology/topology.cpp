#include <stdexcept>
#include <fmt/core.h>
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

switch_id_t SimpleTopology::Topology::getTorId(switch_id_t dstHostId){

    // Even numbered hosts on ToR 1. Odd numbered on Tor 0.
    return (dstHostId + 1) % 2;
}

void SimpleTopology::Topology::buildTopo(){

}
