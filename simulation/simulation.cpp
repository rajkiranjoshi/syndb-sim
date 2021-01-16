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

    debug_print_yellow("Inside process hosts");
    debug_print("Num hosts: {}", this->topo.hostIDMap.size());

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
    
    routeScheduleInfo rinfo;

    // List of iterators that we would delete in the end
    std::list<std::list<pktevent_p<triggerpkt_p>>::iterator> toDelete;

    toDelete.clear(); // Making sure that the list is empty

    auto it = this->TriggerPktEventList.begin();

    while (it != this->TriggerPktEventList.end())
    {
        pktevent_p<triggerpkt_p> event = *it;

        if(this->currTime >= event->pktForwardTime){

            // Handling the case that the next hop is the pkt's dstSwitch
            if(event->nextSwitch->id == event->pkt->dstSwitchId){
                
                // Pass the pkt to the next switch to handle
                event->nextSwitch->receiveTriggerPkt(event->pkt); // can parallelize switch's processing? 

                // Add the event's iterator to the toDelete list
                toDelete.push_front(it); 
            }
            else // forward the event's packet
            {
                status_t status = event->nextSwitch->routeTriggerPkt(event->pkt, rinfo);

                if(status != SUCCESS){
                    std::string msg = fmt::format("Simulator failed to route trigger pkt {} of trigger event", event->pkt->id);
                    throw std::logic_error(msg);
                }

                if(rinfo.nextHopType == HostNode){
                    std::string msg = fmt::format("INCORRECT routing. NextHop for trigger pkt {} is a host!!", event->pkt->id);
                    throw std::logic_error(msg);
                }

                event->doForwarding(rinfo);
                
            }                
        }

        it++;  // iterating over TriggerPktEventList
    }

    // Now delete the enlisted TriggerPktEvents

    debug_print(fmt::format("Deleting {} TriggerPktEvents...", toDelete.size()));
    auto it2 = toDelete.begin();

    while (it2 != toDelete.end()){
        TriggerPktEventList.erase(*it2);
    }

    it2++;
}


void Simulation::processNormalPktEvents(){

    host_tor_link_p pktNextLink;
    switch_p pktNextSwitch;
    sim_time_t pktNextSendTime, timeAfterSwitchHop, pktNextSerializeStartTime;
    routeScheduleInfo rinfo;

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
                auto it = syndbSim.pktLatencyMap.find(event->pkt->id);
                (it->second).end_time = event->pktForwardTime;
                #endif
            }
            // Handling the case that the next hop is the dst ToR switch
            else if(event->nextSwitch->id == syndbSim.topo.getTorId(event->pkt->dstHost)){
                
                // Pass the pkt to the next switch to handle
                event->nextSwitch->receiveNormalPkt(event->pkt); // can parallelize switch's processing?

                /* Calculate the pkt forward time on the ToR --> Host link */
                pktNextSwitch = event->nextSwitch;
                pktNextLink = pktNextSwitch->neighborHostTable[event->pkt->dstHost];
                
                // time when we can start serialization *earliest* on the next switch
                timeAfterSwitchHop = event->pktForwardTime + event->nextSwitch->hop_delay;
                // actual time when we can start serialization assuming FIFO queuing on the next link
                pktNextSerializeStartTime = std::max<sim_time_t>(timeAfterSwitchHop, pktNextLink->next_idle_time_to_host);

                // Time when serialization would end and pkt can be forwarded to dst host
                pktNextSendTime = pktNextSerializeStartTime + getSerializationDelay(event->pkt->size, pktNextLink->speed);

                // Schedule the packet on the Tor-Host link
                pktNextLink->next_idle_time_to_host = pktNextSendTime;

                /* Update the event */
                event->pktForwardTime = pktNextSendTime; 
                event->currSwitch = event->nextSwitch;
                event->nextSwitch = NULL;

            }
            // forward the event's packet (switch-to-switch fowarding)
            // next switch is NOT a dstTor switch. So need to invoke routing
            else 
            {
                event->nextSwitch->receiveNormalPkt(event->pkt); // can parallelize switch's processing? 
                status_t status = event->nextSwitch->routeNormalPkt(event->pkt, rinfo);

                if(status != SUCCESS){
                    std::string msg = fmt::format("Simulator failed to route normal pkt {} of normal event", event->pkt->id);
                    throw std::logic_error(msg);
                }

                if(rinfo.nextHopType == HostNode){
                    std::string msg = fmt::format("INCORRECT routing. NextHop is surely NOT a host. But routing returned a host!!", event->pkt->id);
                    throw std::logic_error(msg);
                }

                event->doForwarding(rinfo);
                
            }                
        }

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
    
    // Why this is needed? When std::list is destroyed, if its members are pointers, the members are not destroyed.
    // BUT, what about shared_ptrs?

    // Clean-up the members of the syndbSim

    // Clean-up the topo
}
