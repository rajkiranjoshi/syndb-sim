#include "topology/fattree_topology.hpp"
#include "utils/logger.hpp"

void Pod::buildPod(){

   
    // Construct the racks
    auto tor_it = this->torSwitches.begin();
    for(tor_it; tor_it != this->torSwitches.end(); tor_it++){
        
        // create the ToR switch
        *tor_it = parentTopo.createNewSwitch(SwitchType::FtTor);

        // Update podId for the ToR switch
        std::dynamic_pointer_cast<SwitchFtTor>(*tor_it)->podId = this->id;

        // create and add k/2 hosts to the ToR switch
        for(int i=0; i < (syndbConfig.fatTreeTopoK / 2); i++){
            host_p h = parentTopo.createNewHost();
            parentTopo.addHostToTor(h, *tor_it);
        }

    } // end of tor_it loop

     // Initialize the Aggr switches
    // racklocal_host_id_t localHostId = 0;
    auto aggr_it = this->aggrSwitches.begin();
    for(aggr_it; aggr_it != this->aggrSwitches.end(); aggr_it++){
        *aggr_it = parentTopo.createNewSwitch(SwitchType::FtAggr);
        std::dynamic_pointer_cast<SwitchFtAggr>(*aggr_it)->podId = this->id;

        // connect the Aggr switch to the ToR switches
        auto tor_it = this->torSwitches.begin();
        for(tor_it; tor_it != this->torSwitches.end(); tor_it++){
            // Adds links, updates neighborSwitch tables on both the switches
            parentTopo.connectSwitchToSwitch(*tor_it, *aggr_it);
        }
    }

    
    // Populate routing tables for the ToR switches
    ft_scale_t ftscaleK = syndbConfig.fatTreeTopoK;
    uint aggrSwitchIdx;

    for(int z=0; z < this->torSwitches.size(); z++){ // loops over ToR switches
        std::shared_ptr<SwitchFtTor> tor = std::dynamic_pointer_cast<SwitchFtTor>(this->torSwitches[z]);

        // Fills routing table of tor for all possible racklocal host IDs
        for(int rlocalHostId=0; rlocalHostId < ftscaleK/2; rlocalHostId++){ 
            aggrSwitchIdx = (rlocalHostId + z) % (ftscaleK/2);
            tor->routingTable[rlocalHostId] = this->aggrSwitches[aggrSwitchIdx]->id;
        }
    }

}

void FattreeTopology::buildTopo(){

    // Instantiate and build the pods 
    for(int i=0; i < this->k; i++){
        this->pods[i] = pod_p(new Pod(this->getNextPodId(),*this));
        this->pods[i]->buildPod();
    }

    // Create all Core switches
    for(int i=0; i < this->numCoreSwitches; i++){
        this->coreSwitches[i] = this->createNewSwitch(SwitchType::FtCore);
    }

    // Connect Core switches to the pods. Update routing on only the Core switches
    /* 
        For each core switch, compute the stride number it belongs to. 
        Then for each pod, index the aggrSwitch array with the stride number 
        to get the connected aggr switch.
    */
    for(uint coreSwitchIdx=0; coreSwitchIdx < this->numCoreSwitches; coreSwitchIdx++){
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

            // Add the core switch to AggrSwitch's coreSwitchesList
            std::dynamic_pointer_cast<SwitchFtAggr>(aggrSwitch)->coreSwitchesList.push_back(coreSwitch->id);
          
        }    
        
    } // end of for loop on all core switches


    // Update routing on all Aggr switches (across all pods)
    ft_scale_t ftscaleK = syndbConfig.fatTreeTopoK;
    uint coreSwitchIdx;
    for(int i=0; i < this->pods.size(); i++){ // loops over all pods
        pod_p pod = this->pods[i];

        for(int z=0; z < pod->aggrSwitches.size(); z++){ // loops over aggr switches in the pod
            std::shared_ptr<SwitchFtAggr> aggrSwitch = std::dynamic_pointer_cast<SwitchFtAggr>(pod->aggrSwitches[z]);

            // Fill entries in the routing table of the aggrSwitch
            for(int rlocalHostId=0; rlocalHostId < ftscaleK/2; rlocalHostId++){
                coreSwitchIdx = (rlocalHostId + z) % (ftscaleK/2);
                aggrSwitch->routingTable[rlocalHostId] = aggrSwitch->coreSwitchesList[coreSwitchIdx];
            } // end of routing table entries   
        } // end of aggrSwitches in the pod
    } // end of loop over all pods
    
}
