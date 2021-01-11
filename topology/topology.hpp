#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <vector>
#include "topology/host.hpp"
#include "topology/link.hpp"
#include "topology/switch.hpp"

typedef struct Topology
{
    std::vector<host_p> hostList;
    std::vector<link_p> linkList;
    std::vector<switch_p> switchList;

    virtual switch_id_t getTorId(switch_id_t dstHostId);
    virtual void buildTopo();
  
} Topology;


typedef struct SimpleTopology : Topology
{
    // switch_id_t getTorId(switch_id_t dstHostId);
} SimpleTopology;



#endif
