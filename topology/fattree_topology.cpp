#include "topology/fattree_topology.hpp"

void Pod::buildPod(){

    // Common routingTable for all ToRs
    std::unordered_map<racklocal_host_id_t, switch_id_t> commonTorRoutingTable;

    // Initialize the Aggr switches & populate common ToR routing table
    racklocal_host_id_t localHostId = 0;
    auto aggr_it = this->aggrSwitches.begin();
    for(aggr_it; aggr_it != this->aggrSwitches.end(); aggr_it++){
        *aggr_it = parentTopo.createNewSwitch(SwitchType::FtAggr);
        std::dynamic_pointer_cast<SwitchFtAggr>(*aggr_it)->podId = this->id;

        commonTorRoutingTable[localHostId++] = (*aggr_it)->id;
    }


    // Construct the racks and connect them to Aggr switches
    auto tor_it = this->torSwitches.begin();
    for(tor_it; tor_it != this->torSwitches.end(); tor_it++){
        
        // create the ToR switch
        *tor_it = parentTopo.createNewSwitch(SwitchType::FtTor);

        // Update podId for the ToR switch
        std::dynamic_pointer_cast<SwitchFtTor>(*tor_it)->podId = this->id;
        // Update routingTable for the ToR switch
        std::dynamic_pointer_cast<SwitchFtTor>(*tor_it)->routingTable = commonTorRoutingTable;

        // create and add k/2 hosts to the ToR switch
        for(int i=0; i < (syndbConfig.fatTreeTopoK / 2); i++){
            host_p h = parentTopo.createNewHost();
            parentTopo.addHostToTor(h, *tor_it);
        }

        // connect the ToR switch to the Aggr switches
        auto aggr_it = this->aggrSwitches.begin();
        for(aggr_it; aggr_it != this->aggrSwitches.end(); aggr_it++){
            // Adds links, updates neighborSwitch tables on both the switches
            parentTopo.connectSwitchToSwitch(*tor_it, *aggr_it);
        }

    } // end of tor_it loop

}

void FattreeTopology::buildTopo(){

    // Instantiate and build the pods 
    for(int i=0; i < this->k; i++){
        this->pods[i] = pod_p(new Pod(this->getNextPodId(),*this));
        this->pods[i]->buildPod();
    }

    // Create all Core switches
    for(int i=0; i < this->numCoreSwitches; i++){
        this->coreSwitches[i] = switch_p(new SwitchFtCore(this->getNextSwitchId()));
    }

    // Connect Core switches to the pods. Update routing on both the switches
    /* 
        For each core switch, compute the stride number it belongs to. 
        Then for each pod, index the aggrSwitch array with the stride number 
        to get the connected aggr switch.
    */
    for(uint coreSwitchIdx=0; coreSwitchIdx < this->k; coreSwitchIdx++){
        // compute the stride for the core switch
        uint stride = coreSwitchIdx / (this->k/2); 
        uint strideLocalCoreSwitchIdx = coreSwitchIdx % (this->k/2); 
        switch_p coreSwitch = this->coreSwitches[coreSwitchIdx];

        for(pod_id_t podIdx=0; podIdx < this->k; podIdx++){
            switch_p aggrSwitch = this->pods[podIdx]->aggrSwitches[stride];

            // Add links, update neighborSwitch tables on both the switches
            this->connectSwitchToSwitch(coreSwitch, aggrSwitch);
            // Update routing table on the Core Switch
            std::dynamic_pointer_cast<SwitchFtCore>(coreSwitch)->routingTable[podIdx] = aggrSwitch->id;
            // Update routing table on the Aggr Switch
            std::dynamic_pointer_cast<SwitchFtAggr>(aggrSwitch)->routingTable[strideLocalCoreSwitchIdx] = coreSwitch->id;
        }    
        
    }
    
}
