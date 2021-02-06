#include <iostream>
#include <vector>

#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "data-analysis/dataparser.hpp"

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

    ndebug_print_yellow("Welcome to data analysis!");

    Config syndbConfig;
    DataParser dataparser(PREFIX_STRING_FOR_DATA_FILES, syndbConfig.numSwitches, syndbConfig.numHosts);

    // getRoute(2688,0,24);
    

    // Ignore parameters for now
    switch_id_t triggerSwitchID = 687;
    sim_time_t triggerTime = 22562;
    pkt_id_t windowSize = 10;

    // get p-record window for trigger switch
    std::map<pkt_id_t, PacketInfo> pRecordWindowForTriggerSwitch = dataparser.getWindowForSwitch(triggerSwitchID, triggerTime, windowSize);
    std::map<pkt_id_t, PacketInfo>::iterator iteratorForpRecordWindowForTriggerSwitch = pRecordWindowForTriggerSwitch.begin();

    std::cout<< "-----------------------" << std::endl;
    for (; iteratorForpRecordWindowForTriggerSwitch != pRecordWindowForTriggerSwitch.end(); iteratorForpRecordWindowForTriggerSwitch++) {
        std::cout << iteratorForpRecordWindowForTriggerSwitch->first << "\t" << iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime << std::endl;
    }

/*     std::cout<< "-----------------------" << std::endl;
    for (int iteratorForAllSwitches=0; iteratorForAllSwitches < syndbConfig.numSwitches; iteratorForAllSwitches++) {
        if (iteratorForAllSwitches == triggerSwitchID) {
            continue;
        }
        std::map<pkt_id_t, PacketInfo> pRecordWindowForCurrentSwitch = dataparser.getWindowForSwitch(iteratorForAllSwitches, triggerTime, windowSize);
        std::map<pkt_id_t, PacketInfo>::iterator iteratorForpRecordWindowForCurrentSwitch = pRecordWindowForCurrentSwitch.begin();
        // for (; iteratorForpRecordWindowForCurrentSwitch != pRecordWindowForCurrentSwitch.end(); iteratorForpRecordWindowForCurrentSwitch++) {
        //     std::cout << iteratorForpRecordWindowForCurrentSwitch->first << "\t" << iteratorForpRecordWindowForCurrentSwitch->second.switchIngressTime << std::endl;
        // }

        
        float correlation = dataparser.getCorrelationBetweenPrecordWindows(pRecordWindowForTriggerSwitch, pRecordWindowForCurrentSwitch);
        std::cout<< "Correlation between Switch " << triggerSwitchID << " (trigger) and Switch " << iteratorForAllSwitches << " is: " << correlation << std::endl;
    } */

    return 0;
}



