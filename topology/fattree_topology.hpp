#ifndef FATTREE_TOPOLOGY_H
#define FATTREE_TOPOLOGY_H

#include <array>
#include "simulation/config.hpp"
#include "topology/topology.hpp"

struct Pod;
typedef std::shared_ptr<Pod> pod_p;

struct FattreeTopology : Topology
{
    ft_scale_t k;
    uint numCoreSwitches;
    pod_id_t nextPodId = 0;
    std::array<pod_p, syndbConfig.fatTreeTopoK> pods;
    std::array<switch_p, syndbConfig.numCoreSwitches> coreSwitches;
    std::list<switch_p> aggrSwitches; 
    std::list<switch_p> torSwitches;

    // Override the virtual function of the abstract Topology class
    void buildTopo();
    inline FattreeTopology(ft_scale_t k):Topology(){
        this->k = k;
        this->numCoreSwitches = syndbConfig.numCoreSwitches;
    }
    inline pod_id_t getNextPodId(){ return this->nextPodId++;}
};

struct Pod
{
    pod_id_t id;
    FattreeTopology &parentTopo;

    std::array<switch_p, syndbConfig.fatTreeTopoK/2 > aggrSwitches;
    std::array<switch_p, syndbConfig.fatTreeTopoK/2 > torSwitches;

    Pod(pod_id_t id, FattreeTopology &parent) : parentTopo(parent){
        this->id = id;
    };

    void buildPod();

    private:
    void connectToRtoAggr(switch_p tor, switch_p aggr);
};



#endif
