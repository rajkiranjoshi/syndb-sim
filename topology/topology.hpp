#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <vector>
#include <map>
#include <unordered_map>
#include "topology/host.hpp"
#include "topology/link.hpp"
#include "topology/switch.hpp"
#include "topology/switch_ft.hpp"

typedef std::unordered_map<host_id_t, switch_id_t> host_tor_map_t;
typedef std::unordered_map<host_id_t, host_p> host_id_map_t;
typedef std::unordered_map<switch_id_t, switch_p> switch_id_map_t;


struct Topology
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
    std::map<SwitchType, std::set<switch_id_t>> switchTypeIDMap; // updated by createNewSwitch()

    switch_id_t getTorId(host_id_t hostId);
    Switch* getSwitchById(switch_id_t id);
    SwitchType getSwitchTypeById(switch_id_t id); 
    Host* getHostById(host_id_t hostId);

    inline host_id_t getNextHostId() {return this->nextHostId++;}
    inline link_id_t getNextLinkId() {return this->nextLinkId++;}
    inline switch_id_t getNextSwitchId() {return this->nextSwitchId++;}

    host_tor_link_p createNewToRLink();
    network_link_p createNewNetworLink(switch_id_t sw1, switch_id_t sw2);
    switch_p createNewSwitch(SwitchType type);
    host_p createNewHost(bool trafficGenDisabled = false);

    void addHostToTor(host_p &host, switch_p &tor);
    void connectSwitchToSwitch(switch_p &s1, switch_p &s2);

    Topology();
    virtual void buildTopo() = 0;
  
};

/* 
           s2
        /      \
       s0       s1
       |        |
       h0       h1

*/
struct SimpleTopology : Topology
{
    // Override the virtual function of the abstract class
    void buildTopo();
};

/* 
    h0 --- s0 --- s1 --- s2 --- s3 --- s4 --- h1
*/
struct LineTopology : Topology
{
    // Override the virtual function of the abstract class
    void buildTopo();
};



#endif
