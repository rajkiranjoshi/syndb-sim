#include "topology/topology.hpp"


switch_id_t SimpleTopology::getTorId(switch_id_t dstHostId){

    // Even numbered hosts on ToR 1. Odd numbered on Tor 0.
    return (dstHostId + 1) % 2;
}