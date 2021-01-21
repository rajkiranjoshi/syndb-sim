#include <string>
#include <fmt/core.h>

#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "utils/logger.hpp"
#include "utils/utils.hpp"


Simulation syndbSim;

Simulation::Simulation(){

    this->currTime = 0;
    this->timeIncrement = syndbConfig.timeIncrementNs;
    this->totalTime = (sim_time_t)(syndbConfig.totalTimeMSecs * (float)1000000);

    if(syndbConfig.topo == "SimpleToplogy"){
        this->topo = SimpleTopology();
    }

    this->nextPktId = 0;
    this->nextTriggerPktId = 0;

}

void Simulation::initHosts(){

    auto it = this->topo.hostIDMap.begin();

    while (it != this->topo.hostIDMap.end() )
    {
        host_p h = it->second;
        h->generateNextPkt();

        it++;
    }

    debug_print(fmt::format("Initialized {} hosts!", this->topo.hostIDMap.size()));
    
}

void Simulation::processHosts(){

    // debug_print_yellow("Inside process hosts");
    // debug_print("Num hosts: {}", this->topo.hostIDMap.size());

    auto it = this->topo.hostIDMap.begin();

    while (it != this->topo.hostIDMap.end() )
    {
        host_p h = it->second;

        if(this->currTime >= h->nextPktTime){
            h->sendPkt();
        }
        

        it++;
    }
}



void Simulation::processTriggerPktEvents(){
    
    routeScheduleInfo rsinfo;

    // List of iterators that we would delete in the end
    std::list<std::list<pktevent_p<triggerpkt_p>>::iterator> toDelete;

    toDelete.clear(); // Making sure that the list is empty

    auto it = this->TriggerPktEventList.begin();

    while (it != this->TriggerPktEventList.end())
    {
        pktevent_p<triggerpkt_p> event = *it;

        if(this->currTime >= event->pktForwardTime){

            // Pass the pkt to the next switch to handle
            event->nextSwitch->receiveTriggerPkt(event->pkt, event->pktForwardTime); // can parallelize switch's processing? 

            // Handling the case that the next hop is the pkt's dstSwitch
            if(event->nextSwitch->id == event->pkt->dstSwitchId){
                // The pkt requires no more network event processing
                // Add the event's iterator to the toDelete list
                toDelete.push_front(it); 
            }
            else // forward the event's packet
            {
                // Call routing on the next switch
                syndb_status_t status = event->nextSwitch->routeScheduleTriggerPkt(event->pkt, event->pktForwardTime, rsinfo);

                if(status != syndb_status_t::success){
                    std::string msg = fmt::format("Simulator failed to route trigger pkt of trigger event {}", event->pkt->triggerId);
                    throw std::logic_error(msg);
                }

                // event->doForwarding(rinfo);

                event->currSwitch = event->nextSwitch;
                event->nextSwitch = rsinfo.nextSwitch;
                event->pktForwardTime = rsinfo.pktNextForwardTime;
                
            }                
        }

        it++;  // iterating over TriggerPktEventList
    }

    // Now delete the enlisted TriggerPktEvents

    debug_print(fmt::format("Deleting {} TriggerPktEvents...", toDelete.size()));
    auto it2 = toDelete.begin();

    while (it2 != toDelete.end()){
        TriggerPktEventList.erase(*it2);

        it2++;
    }

    
}


void Simulation::processNormalPktEvents(){

    routeScheduleInfo rsinfo;

    // List of iterators that we would delete in the end
    std::list<std::list<pktevent_p<normalpkt_p>>::iterator> toDelete;

    toDelete.clear(); // Making sure that the list is empty

    auto it = this->NormalPktEventList.begin();

    while (it != this->NormalPktEventList.end())
    {
        pktevent_p<normalpkt_p> event = *it;

        if(this->currTime >= event->pktForwardTime){

            
            // Handling the case that the next hop is the dst Host
            if(event->nextSwitch == NULL){
                // Mark the event for deletion
                toDelete.push_back(it);
                
                // For devtest testNormalPktLatencies()
                #ifdef DEBUG
                auto it = syndbSim.NormalPktLatencyMap.find(event->pkt->id);
                (it->second).end_time = event->pktForwardTime;
                #endif

                // TODO: check if pkt is updated with all metadata
                syndbSim.pktDumper.dumpPacket(event->pkt);
            }
            // Handling the case that the next hop is a switch (intermediate or dstTor)
            else{
                
                // Pass the pkt to the next switch to handle
                event->nextSwitch->receiveNormalPkt(event->pkt, event->pktForwardTime); // can parallelize switch's processing?

                // Call routing on the next switch
                syndb_status_t status = event->nextSwitch->routeScheduleNormalPkt(event->pkt, event->pktForwardTime, rsinfo);
                
                
                /* Update the event */
                event->currSwitch = event->nextSwitch;
                event->nextSwitch = rsinfo.nextSwitch; // will be NULL if next hop is a host
                event->pktForwardTime = rsinfo.pktNextForwardTime;               

            }

        } // end of the if(this->currTime >= event->pktForwardTime)

        it++;  // iterating over NormalPktEventList
    } 

    // Now delete the enlisted NormalPktEvents

    debug_print(fmt::format("Deleting {} NormalPktEvents...", toDelete.size()));
    auto it2 = toDelete.begin();

    while (it2 != toDelete.end()){
        NormalPktEventList.erase(*it2);
        it2++;
    }


}




void Simulation::cleanUp(){
    
    // Why this is needed? When std::list is destroyed, if its members are pointers, only the pointers are destroyed, not the objects pointed by the pointers.
    // BUT for shared_ptrs, when they are deleted, they do destruct the member objects.
    // SO this function is actually NOT really required.

    // Clean-up the members of the syndbSim

    // Clean-up the topo
}
