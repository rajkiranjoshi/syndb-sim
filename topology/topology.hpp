#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <vector>
#include <unordered_map>
#include "topology/host.hpp"
#include "topology/link.hpp"
#include "topology/switch.hpp"

typedef std::unordered_map<host_id_t, switch_id_t> host_tor_map_t;
typedef std::pair<host_id_t, switch_id_t> host_tor_map_pair;

typedef std::unordered_map<host_id_t, host_p> host_id_map_t;
typedef std::pair<host_id_t, host_p> host_id_map_pair;

typedef std::unordered_map<switch_id_t, switch_p> switch_id_map_t;
typedef std::pair<switch_id_t, switch_p> switch_id_map_pair;

typedef struct Topology
{
    host_id_t nextHostId = 0;
    link_id_t nextLinkId = 0;
    switch_id_t nextSwitchId = 0;

    // Hold all shared_ptrs so that hosts, links, switches don't get destroyed by mistake
    // Keeping two links separate for possible future optimizations
    std::vector<network_link_p> networkLinkVector;
    std::vector<host_tor_link_p> torLinkVector;
    host_id_map_t hostIDMap;   
    switch_id_map_t switchIDMap;
    
    host_tor_map_t hostTorMap; // updated by addHostToTor()

    switch_id_t getTorId(host_id_t hostId);
    virtual void buildTopo();
    switch_p getSwitchById(switch_id_t id);

    private:
    inline host_id_t getNextHostId() {return this->nextHostId++;}
    inline link_id_t getNextLinkId() {return this->nextLinkId++;}
    inline switch_id_t getNextSwitchId() {return this->nextSwitchId++;}

    host_tor_link_p createNewToRLink();
    network_link_p createNewNetworLink(switch_id_t sw1, switch_id_t sw2);
    switch_p createNewSwitch();
    host_p createNewHost();

    void addHostToTor(host_p host, switch_p tor);
    void connectSwitchToSwitch(switch_p s1, switch_p s2);
  
} Topology;


typedef struct SimpleTopology : Topology
{
    // switch_id_t getTorId(switch_id_t dstHostId);
} SimpleTopology;



#endif
