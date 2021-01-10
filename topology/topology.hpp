#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <vector>
#include "topology/host.hpp"
#include "topology/link.hpp"
#include "topology/switch.hpp"

typedef struct Topology
{
    std::vector<Host> hostList;
    std::vector<Link> linkList;
    std::vector<Switch> switchList;

    virtual switch_id_t getTorId(switch_id_t dstHostId) = 0;
  
} Topology;


typedef struct SimpleTopology : Topology
{
    switch_id_t getTorId(switch_id_t dstHostId);
} SimpleTopology;





#endif