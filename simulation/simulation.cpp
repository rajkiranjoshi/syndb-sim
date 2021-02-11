#include <string>
#include <time.h>
#include <fmt/core.h>
#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "utils/logger.hpp"
#include "utils/utils.hpp"
#include "utils/pktdumper.hpp"
#include "topology/fattree_topology.hpp"


Simulation syndbSim;

Simulation::Simulation(){

    this->startTime = time(NULL); 
    this->currTime = 0;
    this->timeIncrement = syndbConfig.timeIncrementNs;
    this->totalTime = (sim_time_t)(syndbConfig.totalTimeMSecs * (float)1000000);

    if(syndbConfig.topoType == TopologyType::Simple){
        this->topo = std::shared_ptr<Topology>(new SimpleTopology());
    }
    else if (syndbConfig.topoType == TopologyType::FatTree){
        this->topo = std::shared_ptr<Topology>(new FattreeTopology(syndbConfig.fatTreeTopoK));
    }

    this->nextPktId = 0;
    this->nextTriggerPktId = 0;
    this->totalPktsDelivered = 0;

    #if LOGGING
    this->pktDumper = std::unique_ptr<PktDumper>(new PktDumper());
    #endif
    
}


pktevent_p<normalpkt_p> Simulation::getNewNormalPktEvent(){
    
    pktevent_p<normalpkt_p> newNormalPktEvent;

    if(this->freeNormalPktEvents.size() > 0){
        newNormalPktEvent = std::move(*this->freeNormalPktEvents.begin()); // retrieve
        this->freeNormalPktEvents.pop_front(); // remove
    }
    else{
        newNormalPktEvent = std::make_shared<PktEvent<normalpkt_p>>();
    }

    return std::move(newNormalPktEvent);
}

normalpkt_p Simulation::getNewNormalPkt(pkt_id_t pktId, pkt_size_t pktSize){
    normalpkt_p newNormalPkt;

    if(this->freeNormalPkts.size() > 0){
        newNormalPkt = *this->freeNormalPkts.begin(); // retrieve
        this->freeNormalPkts.pop_front(); // remove

        newNormalPkt->id = pktId;
        newNormalPkt->size = pktSize;
        newNormalPkt->switchINTInfoList.clear();
    }
    else{
        newNormalPkt = new NormalPkt(pktId, pktSize);
    }

    return newNormalPkt;
}

void Simulation::initTriggerGen(){
    switch(syndbConfig.topoType){
        case TopologyType::Simple:
            this->triggerGen = std::shared_ptr<TriggerGenerator>(new TriggerGeneratorSimpleTopo());
            break;
        case TopologyType::FatTree:
            this->triggerGen = std::shared_ptr<TriggerGenerator>(new TriggerGeneratorFatTreeTopo());
            break;
    }
}

void Simulation::initIncastGen(){
    this->incastGen = std::shared_ptr<IncastGenerator>(new IncastGenerator());
}

void Simulation::initHosts(){

    auto it = this->topo->hostIDMap.begin();

    while (it != this->topo->hostIDMap.end() )
    {
        Host* h = it->second.get();

        if(syndbConfig.trafficPatternType == TrafficPatternType::FtMixed){
            std::dynamic_pointer_cast<FtMixedTrafficPattern>(h->trafficPattern)->initTopoInfo();
        }

        h->generateNextPkt();

        it++;
    }

    ndebug_print(fmt::format("Initialized {} hosts", this->topo->hostIDMap.size()));
    
}

void Simulation::generateHostPktEvents(){
    
    // HostPktEventList MUST be empty
    assert((this->HostPktEventList.size() == 0) && "HostPktEventList is NOT empty!");

    for(auto it = this->topo->hostIDMap.begin(); it != this->topo->hostIDMap.end(); it++){
        Host* host = it->second.get();

        #ifdef DEBUG
        // Just for debug case when trafficGen is disabled
        if (host->trafficGenDisabled)
            continue;
            
        #endif
        
        while(host->nextPktTime <= syndbSim.currTime + syndbSim.timeIncrement){
            // Use the next scheduled packet on the host to create hostPktEvent
            // hostpktevent_p hostPktEvent = hostpktevent_p(new HostPktEvent(host, host->nextPkt));

            // Insert the hostPktEvent into the map (sorted list)
            this->HostPktEventList.insert(std::pair<sim_time_t, HostPktEvent>(host->nextPktTime, HostPktEvent(host, host->nextPkt)));

            // Generate next scheduled packet on the host
            host->generateNextPkt();
        }

    }

}

void Simulation::processHostPktEvents(){

    normalpkt_p nextPkt;
    sim_time_t nextPktTime;
    Host* host;

    auto it = this->HostPktEventList.begin();

    while (it != this->HostPktEventList.end() )
    {
        nextPktTime = it->first;
        host = it->second.host;
        nextPkt = it->second.pkt;

        if(this->currTime < nextPktTime){
            std::string msg = fmt::format("Currtime: {}ns. HostPktEventList has pkt with nextPktTime {}ns", this->currTime, nextPktTime);
            throw std::logic_error(msg);
        }

        host->sendPkt(nextPkt, nextPktTime);

        it = this->HostPktEventList.erase(it); // erase and increment iterator

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

    // debug_print(fmt::format("Deleting {} TriggerPktEvents...", toDelete.size()));
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
        PktEvent<normalpkt_p>* event = (*it).get();

        if(this->currTime >= event->pktForwardTime){

            
            // Handling the case that the next hop is the dst Host
            if(event->nextSwitch == NULL){
                // Mark the event for deletion
                toDelete.push_back(it);
                
                // Add end time INT data to the packet
                event->pkt->endTime = event->pktForwardTime;

                // Dump the pkt with INT data to the disk
                #if LOGGING
                syndbSim.pktDumper->dumpPacket(event->pkt);
                #endif
                this->totalPktsDelivered += 1;

                #ifdef DEBUG
                /* debug_print_yellow("\nPkt ID {} dump:", event->pkt->id);
                debug_print("h{} --> h{}: {} ns (Start: {} ns | End: {} ns)", event->pkt->srcHost, event->pkt->dstHost, event->pkt->endTime - event->pkt->startTime, event->pkt->startTime, event->pkt->endTime);
                auto it1 = event->pkt->switchINTInfoList.begin();
                for(it1; it1 != event->pkt->switchINTInfoList.end(); it1++){
                    debug_print("Rx on s{} at {} ns", it1->swId, it1->rxTime);
                } */
                #endif
            }
            // Handling the case that the next hop is a switch (intermediate or dstTor)
            else{
                
                // Pass the pkt to the next switch to handle
                event->nextSwitch->receiveNormalPkt(event->pkt, event->pktForwardTime); // thread-safe

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

    // debug_print(fmt::format("Deleting {} NormalPktEvents...", toDelete.size()));
    auto it2 = toDelete.begin();

    while (it2 != toDelete.end()){
        this->freeNormalPkts.push_back((**it2)->pkt); // this makes pkt inside the event as NULL
        //TODO: other members of the PktEvent?
        this->freeNormalPktEvents.push_back(std::move(**it2));
        NormalPktEventList.erase(*it2);
        it2++;
    }


}


void Simulation::flushRemainingNormalPkts(){

    auto it = this->NormalPktEventList.begin();

    for(it; it != this->NormalPktEventList.end(); it++){
        pktevent_p<normalpkt_p> event = *it;

        // Dump the pkt with INT data to the disk
        #if LOGGING
        syndbSim.pktDumper->dumpPacket(event->pkt);
        #endif


        #ifdef DEBUG
        /* debug_print_yellow("\nPkt ID {} dump:", event->pkt->id);
        debug_print("h{} --> h{}: N/A ns (Start: {} ns | End: {} ns)", event->pkt->srcHost, event->pkt->dstHost, event->pkt->startTime, event->pkt->endTime);
        auto it1 = event->pkt->switchINTInfoList.begin();
        for(it1; it1 != event->pkt->switchINTInfoList.end(); it1++){
            debug_print("Rx on s{} at {} ns", it1->swId, it1->rxTime);
        } */
        #endif
    }

}

void Simulation::logTriggerInfoMap(){
    sim_time_t triggerOriginTime, rxTime; 
    trigger_id_t triggerId;
    switch_id_t originSwitch, rxSwitch;

    auto it1 = syndbSim.TriggerInfoMap.begin();

    debug_print_yellow("\nTrigger pkt latencies between switches");
    for(it1; it1 != syndbSim.TriggerInfoMap.end(); it1++){
        
        triggerId = it1->first;
        originSwitch = it1->second.originSwitch;
        SwitchType switchType = syndbSim.topo->getSwitchTypeById(originSwitch);

        #if LOGGING
        syndbSim.pktDumper->dumpTriggerInfo(triggerId, it1->second, switchType);
        #else
        /* Below code is only for debugging. TODO: comment out later. */

        /*
        triggerOriginTime = it1->second.triggerOrigTime;
        
        ndebug_print_yellow("Trigger ID {} (origin switch: {} {})", triggerId, originSwitch, switchTypeToString(switchType));
        auto it2 = it1->second.rxSwitchTimes.begin();

        for(it2; it2 != it1->second.rxSwitchTimes.end(); it2++){
            rxSwitch = it2->first;
            rxTime = it2->second;

            ndebug_print("{} --> {}: {}ns", originSwitch, rxSwitch, rxTime - triggerOriginTime);
        } // end of iterating over rxSwitchTimes
         */
        #endif


    } // end of iterating over TriggerPktLatencyMap
}

void Simulation::showLinkUtilizations(){

    double util_to_tor, util_to_host;
    double util1, util2;
    double percent_util1, percent_util2;
    double percent_util_to_tor, percent_util_to_host;
    double torLinksPercentUtilSum = 0;
    double networkLinksPercentUtilSum = 0;
    link_id_t numTorLinks = 0; 
    link_id_t numNetworkLinks = 0; 

    // For dumping individual link utilization
    std::string torLinkUtilsFileName = "";
    std::string networkLinkUtilsFileName = "";
    #if LOGGING
    torLinkUtilsFileName = fmt::format("./data/{}torLinksUtil.txt", this->pktDumper->prefixStringForFileName);
    networkLinkUtilsFileName = fmt::format("./data/{}networkLinksUtil.txt", this->pktDumper->prefixStringForFileName);
    #else
    torLinkUtilsFileName = "./data/torLinksUtil.txt";
    networkLinkUtilsFileName = "./data/networkLinksUtil.txt";
    #endif

    std::ofstream torLinkUtilsFile(torLinkUtilsFileName);
    std::ofstream networkLinkUtilsFile(networkLinkUtilsFileName);

    debug_print_yellow("Utilization on ToR links:");
    for(auto it = syndbSim.topo->torLinkVector.begin(); it != syndbSim.topo->torLinkVector.end(); it++){
        
        util_to_tor = (double)((*it)->byte_count_to_tor * 8) / syndbSim.totalTime;
        util_to_host = (double)((*it)->byte_count_to_host * 8) / syndbSim.totalTime;

        percent_util_to_tor = (util_to_tor / syndbConfig.torLinkSpeedGbps) * 100.0;
        percent_util_to_host = (util_to_host / syndbConfig.torLinkSpeedGbps) * 100.0;

        debug_print("Link ID {}: towards host: {} | towards tor: {}", (*it)->id, percent_util_to_host, percent_util_to_tor);

        torLinksPercentUtilSum += percent_util_to_tor;
        torLinksPercentUtilSum += percent_util_to_host;
        numTorLinks += 2;

        // Dumping individual link utilization
        torLinkUtilsFile << percent_util_to_tor << std::endl;
        torLinkUtilsFile << percent_util_to_host << std::endl;
    }

    debug_print_yellow("Utilization on Network links:");
    for(auto it = syndbSim.topo->networkLinkVector.begin(); it != syndbSim.topo->networkLinkVector.end(); it++){
        
        auto map = (*it)->byte_count;
        auto it_byte_count = map.begin();

        switch_id_t sw1 = it_byte_count->first;
        byte_count_t byteCount1 = it_byte_count->second;
        it_byte_count++;
        switch_id_t sw2 = it_byte_count->first;
        byte_count_t byteCount2 = it_byte_count->second;

        util1 = (double)(byteCount1 * 8) / syndbSim.totalTime;
        util2 = (double)(byteCount2 * 8) / syndbSim.totalTime;

        percent_util1 = (util1 / syndbConfig.networkLinkSpeedGbps) * 100.0;
        percent_util2 = (util2 / syndbConfig.networkLinkSpeedGbps) * 100.0;

        debug_print("Link ID {}: towards sw{}: {} | towards sw{}: {}", (*it)->id, sw1, percent_util1, sw2, percent_util2);

        networkLinksPercentUtilSum += percent_util1;
        networkLinksPercentUtilSum += percent_util2;
        numNetworkLinks += 2;

        // Dumping individual link utilization
        networkLinkUtilsFile << percent_util1 << std::endl;
        networkLinkUtilsFile << percent_util2 << std::endl;
    }

    // Closing individual link utilization files
    torLinkUtilsFile.close();
    networkLinkUtilsFile.close();

    ndebug_print_yellow("#####  Network load summary  #####");
    ndebug_print("ToR Links: {}", torLinksPercentUtilSum / numTorLinks);
    ndebug_print("Network Links: {}", networkLinksPercentUtilSum / numNetworkLinks);

}

void Simulation::printSimulationStats(){
    syndbSim.showLinkUtilizations();
    ndebug_print_yellow("#####  Total Pkts Summary  #####");
    ndebug_print("Generated: {} | Delivered: {}", this->nextPktId, this->totalPktsDelivered);
}


void Simulation::printSimulationSetup(){
     
    ndebug_print_yellow("########  Simulation Setup  ########");
    ndebug_print("TrafficPattern type is {}", trafficPatternTypeToString(syndbConfig.trafficPatternType));
    ndebug_print("TrafficDistribution type is {}", trafficGenTypeToString(syndbConfig.trafficGenType));
    ndebug_print("Trigger initial delay is {}ns", syndbConfig.triggerInitialDelay);
    #if LOGGING
        ndebug_print("Logging is enabled!");
    #else
        ndebug_print("Logging is disabled!");
    #endif

    #if HOP_DELAY_NOISE
        ndebug_print("HopDelayNoise is enabled!");
    #else
        ndebug_print("HopDelayNoise is disabled!");
    #endif

    #if TRIGGERS_ENABLED
        ndebug_print("Triggers are enabled!");
    #else
        ndebug_print("Triggers are disabled!");
    #endif

    #if INCASTS_ENABLED
        ndebug_print("Incasts are enabled!");
    #else
        ndebug_print("Incasts are disabled!");
    #endif

    ndebug_print("Time increment is {}ns", syndbSim.timeIncrement);
    ndebug_print("Running simulation for {}ns ...\n",syndbSim.totalTime);
}

void Simulation::cleanUp(){

    ndebug_print_yellow("Flushing remaining normal pkts");
    this->flushRemainingNormalPkts();
    ndebug_print_yellow("Flushing trigger pkts info");
    this->logTriggerInfoMap();

    /* Free NormalPkts from everywhere */
    // 1. From the Hosts. Implemented in the destructor ~Host(). Should be freed there.
    // 2. From the HostPktEventList. Do NOT free in the destructor of HostPktEvent. It is destroyed in runtime.
    for(auto it=this->HostPktEventList.begin(); it != this->HostPktEventList.end(); it++){
        if(it->second.pkt != NULL)
            delete it->second.pkt;
    }
    // 3. From the NormalPktEvents only. NOT the freeNormalPktEvents!
    for(auto it=this->NormalPktEventList.begin(); it != this->NormalPktEventList.end(); it++){
        if((*it)->pkt != NULL)
            delete (*it)->pkt;
    }
    // 4. From the freeNormalPkts list. These are pkts from the freeNormalPktEvents
    ndebug_print_yellow("Cleaning up freeNormalPkts ...");
    for(auto it=this->freeNormalPkts.begin(); it != this->freeNormalPkts.end(); it++){
        if(*it != NULL)
            delete *it;
    }
    
    this->printSimulationStats();

    this->endTime = time(NULL);
    ndebug_print_yellow("\nSimulation run took {} seconds.", this->endTime - this->startTime);

}
