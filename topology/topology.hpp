#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <vector>
#include <unordered_map>
#include "topology/host.hpp"
#include "topology/link.hpp"
#include "topology/switch.hpp"

typedef struct Topology
{
    std::vector<host_p> hostList;
    std::vector<link_p> linkList;
    std::vector<switch_p> switchList;
    std::unordered_map<switch_id_t, switch_p> switchIDMap;

    virtual switch_id_t getTorId(switch_id_t dstHostId);
    virtual void buildTopo();
    switch_p getSwitchById(switch_id_t id);
  
} Topology;


typedef struct SimpleTopology : Topology
{
    // switch_id_t getTorId(switch_id_t dstHostId);
} SimpleTopology;



#endif
