#include <iostream>
#include <vector>

#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "data-analysis/dataparser.hpp"

#define WINDOW_SIZE 100000

std::vector<switch_id_t> getRoute (host_id_t source, host_id_t destination, ft_scale_t fatTreeTopoK) {
    std::vector<switch_id_t> route;

    const uint64_t k = fatTreeTopoK;
    const uint64_t kBy2 = fatTreeTopoK / 2;
    const uint64_t kBy2WholeSquare = kBy2 * kBy2;

    host_id_t sourcePod = source / kBy2WholeSquare;
    host_id_t destinationPod = destination / kBy2WholeSquare;
    #ifdef DEBUG
    debug_print("Source Pod: {}", sourcePod);
    debug_print("Destination Pod: {}", destinationPod);
    #endif

    // get ToR switch ID for first hop
    host_id_t podLocalSourceHostID = source % kBy2WholeSquare;  // between 0 to (kBy2WholeSquare - 1)
    switch_id_t podLocalSourceToRSwitchID = podLocalSourceHostID / kBy2;    // between 0 to (k/2 - 1)
    switch_id_t sourceToRSwitchID = (sourcePod * k) + podLocalSourceToRSwitchID; 
    host_id_t podLocalDestinationHostID = destination % kBy2WholeSquare;  // between 0 to (kBy2WholeSquare - 1)
    switch_id_t podLocalDestinationToRSwitchID = podLocalDestinationHostID / kBy2;    // between 0 to (k/2 - 1)
    switch_id_t destinationToRSwitchID = (destinationPod * k) + podLocalDestinationToRSwitchID; 

    route.push_back(sourceToRSwitchID);

    #ifdef DEBUG
    // debug_print("Pod Local Source ID: {}", podLocalSourceHostID);
    // debug_print("Pod Local Source ToR Switch ID: {}", podLocalSourceToRSwitchID);
    debug_print("Source ToR Switch ID: s{}", sourceToRSwitchID);
    #endif

    if (destinationToRSwitchID == sourceToRSwitchID) {
        return route;
    }

    host_id_t rackLocalDestinationHostID = destination % kBy2;
    switch_id_t podLocalSourceAggregationSwitchID = (sourceToRSwitchID + rackLocalDestinationHostID) % kBy2;   // between 0 to (k/2 - 1)
    switch_id_t sourceAggregationSwitchID = (sourcePod * k) + kBy2 + podLocalSourceAggregationSwitchID;

    route.push_back(sourceAggregationSwitchID);
    #ifdef DEBUG
    // debug_print("Pod Local Source Aggregation Switch ID: {}", podLocalSourceAggregationSwitchID);
    debug_print("Source Aggregation Switch ID: s{}", sourceAggregationSwitchID);
    #endif

    if (destinationPod == sourcePod) {
        #ifdef DEBUG
        // debug_print("Pod Local Destination ID: {}", podLocalDestinationHostID);
        // debug_print("Pod Local Destination ToR Switch ID: {}", podLocalDestinationToRSwitchID);
        debug_print("Destination ToR Switch ID: s{}", destinationToRSwitchID);
        #endif
        route.push_back(destinationToRSwitchID);
        return route;
    }

    switch_id_t localCoreSwitchID = (podLocalSourceAggregationSwitchID + rackLocalDestinationHostID) % kBy2;
    switch_id_t coreSwitchID = (k*k) + (podLocalSourceAggregationSwitchID * kBy2) + localCoreSwitchID;

    route.push_back(coreSwitchID);
    #ifdef DEBUG
    // debug_print("Local Core Switch ID: {}", localCoreSwitchID);
    debug_print("Core Switch ID: s{}", coreSwitchID);
    #endif

    switch_id_t podLocalDestinationAggergationSwitchID = podLocalSourceAggregationSwitchID;
    switch_id_t destinationAggregationSwitchID = (destinationPod * k) + kBy2 + podLocalDestinationAggergationSwitchID;

    route.push_back(destinationAggregationSwitchID);
    route.push_back(destinationToRSwitchID);
    #ifdef DEBUG
    // debug_print("Pod Local Destination Aggregation Switch ID: {}", podLocalDestinationAggergationSwitchID);
    debug_print("Destination Aggregation Switch ID: s{}", destinationAggregationSwitchID);
    // debug_print("Pod Local Destination ID: {}", podLocalDestinationHostID);
    // debug_print("Pod Local Destination ToR Switch ID: {}", podLocalDestinationToRSwitchID);
    debug_print("Destination ToR Switch ID: s{}", destinationToRSwitchID);
    #endif

    return route;
}


int main(){

    ndebug_print_yellow("Welcome to data analysis of {}!", PREFIX_FILE_PATH);

    Config syndbConfig;
    DataParser dataparser(PREFIX_FILE_PATH, PREFIX_STRING_FOR_DATA_FILES, syndbConfig.numSwitches, syndbConfig.numHosts);
    dataparser.getTriggerInfo(syndbConfig.numSwitches);
    
    pkt_id_t windowSize = WINDOW_SIZE;
    
    auto iteratorForTrigger = dataparser.listOfTriggers.begin();
    for (; iteratorForTrigger < dataparser.listOfTriggers.end(); iteratorForTrigger++) {

        switch_id_t triggerSwitchID = iteratorForTrigger->originSwitch;
        // switch_id_t triggerSwitchID = 0;
        sim_time_t triggerTime = iteratorForTrigger->triggerTime;
        ndebug_print_yellow("Trigger ID: {} Switch: {} Time: {}", iteratorForTrigger->triggerId, triggerSwitchID, triggerTime);

        // get p-record window for trigger switch
        std::map<pkt_id_t, PacketInfo> pRecordWindowForTriggerSwitch = dataparser.getWindowForSwitch(triggerSwitchID, triggerTime, windowSize, true);
        
        auto iteratorForpRecordWindowForTriggerSwitch = pRecordWindowForTriggerSwitch.begin();
        ndebug_print_yellow("-----------------------");
        #ifdef DEBUG
        for (; iteratorForpRecordWindowForTriggerSwitch != pRecordWindowForTriggerSwitch.end(); iteratorForpRecordWindowForTriggerSwitch++) {
            ndebug_print("{}\t{}\t{}\t{}", iteratorForpRecordWindowForTriggerSwitch->first,
                                    iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime, 
                                    iteratorForpRecordWindowForTriggerSwitch->second.srcHost, 
                                    iteratorForpRecordWindowForTriggerSwitch->second.dstHost);
        }
        #endif


        for (int switchID = 0; switchID < syndbConfig.numSwitches; switchID++) {
        // for (int switchID = 0; switchID < 20; switchID++) {
            double numberOfExpectedPackets = 0.0, numberOfCommonPackets = 0.0;
            if (switchID == triggerSwitchID) {
                continue;
            }

            // get precord window for switch when it receives the trigger packet
            sim_time_t timeForTriggerPacket = iteratorForTrigger->mapOfSwitchTriggerTime.find(switchID)->second;
            // sim_time_t timeForTriggerPacket = triggerTime;
            debug_print("\tSwitch: {}\t Trigger Packet Time: {}", switchID, timeForTriggerPacket);
            std::map<pkt_id_t, PacketInfo> pRecordWindowForCurrentSwitch = dataparser.getWindowForSwitch(switchID, timeForTriggerPacket, windowSize, false);
            debug_print("Size of precord window: {}", pRecordWindowForCurrentSwitch.size());

            #ifdef DEBUG
            auto iteratorForpRecordWindowForCurrentSwitch = pRecordWindowForCurrentSwitch.begin();
            ndebug_print("-----------------------");
            for (; iteratorForpRecordWindowForCurrentSwitch != pRecordWindowForCurrentSwitch.end(); iteratorForpRecordWindowForCurrentSwitch++) {
                debug_print("{}\t{}", iteratorForpRecordWindowForCurrentSwitch->first,
                                        iteratorForpRecordWindowForCurrentSwitch->second.switchIngressTime);
            }
            #endif

            // for each packet in the trigger switch precord window find the packet in current switch ID
            auto iteratorForpRecordWindowForTriggerSwitch = pRecordWindowForTriggerSwitch.begin();

            for (; iteratorForpRecordWindowForTriggerSwitch != pRecordWindowForTriggerSwitch.end(); iteratorForpRecordWindowForTriggerSwitch++) {
                
                auto route = getRoute(iteratorForpRecordWindowForTriggerSwitch->second.srcHost, 
                                        iteratorForpRecordWindowForTriggerSwitch->second.dstHost, 
                                        syndbConfig.fatTreeTopoK);
                
                auto isSwitchInRoute = std::find(route.begin(), route.end(), switchID);
                if (isSwitchInRoute != route.end()) {
                    debug_print("Expected packet: {}", iteratorForpRecordWindowForTriggerSwitch->first);
                    numberOfExpectedPackets++;
                    auto isPacketInCurrentSwitchID = pRecordWindowForCurrentSwitch.find(iteratorForpRecordWindowForTriggerSwitch->first);
                    if (isPacketInCurrentSwitchID != pRecordWindowForCurrentSwitch.end()) {
                        numberOfCommonPackets++;
                    }
                } 
            }

            if (numberOfExpectedPackets != 0) {
                ndebug_print("\tSwitch ID: {}\t Common Packets: {}\t Correlation: {}%", 
                            switchID, int(numberOfExpectedPackets), 100*numberOfCommonPackets/numberOfExpectedPackets);
            }
        }

    }

    return 0;
}



