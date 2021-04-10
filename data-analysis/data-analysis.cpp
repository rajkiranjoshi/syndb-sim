#include <iostream>
#include <vector>
#include <unordered_set>

#define LOGGING 0
#include "utils/logger.hpp"
#include "data-analysis/dataparser.hpp"


/**
 * Get the route for a given packet in fat-tree topology.
 *
 * @param source        Host ID of the source 
 * @param destination   Host ID of the destination
 * @param fatTreeTopoK  k value for the fat-tree topology
 *
 * @return route        List of switch IDs in the route as a vector
 */
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
    debug_print("Pod Local Source ID: {}", podLocalSourceHostID);
    debug_print("Pod Local Source ToR Switch ID: {}", podLocalSourceToRSwitchID);
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
    debug_print("Pod Local Source Aggregation Switch ID: {}", podLocalSourceAggregationSwitchID);
    debug_print("Source Aggregation Switch ID: s{}", sourceAggregationSwitchID);
#endif

    if (destinationPod == sourcePod) {

#ifdef DEBUG
        debug_print("Pod Local Destination ID: {}", podLocalDestinationHostID);
        debug_print("Pod Local Destination ToR Switch ID: {}", podLocalDestinationToRSwitchID);
        debug_print("Destination ToR Switch ID: s{}", destinationToRSwitchID);
#endif
        route.push_back(destinationToRSwitchID);
        return route;
    }

    switch_id_t localCoreSwitchID = (podLocalSourceAggregationSwitchID + rackLocalDestinationHostID) % kBy2;
    switch_id_t coreSwitchID = (k*k) + (podLocalSourceAggregationSwitchID * kBy2) + localCoreSwitchID;

    route.push_back(coreSwitchID);

#ifdef DEBUG
    debug_print("Local Core Switch ID: {}", localCoreSwitchID);
    debug_print("Core Switch ID: s{}", coreSwitchID);
#endif

    switch_id_t podLocalDestinationAggergationSwitchID = podLocalSourceAggregationSwitchID;
    switch_id_t destinationAggregationSwitchID = (destinationPod * k) + kBy2 + podLocalDestinationAggergationSwitchID;

    route.push_back(destinationAggregationSwitchID);
    route.push_back(destinationToRSwitchID);

#ifdef DEBUG
    debug_print("Pod Local Destination Aggregation Switch ID: {}", podLocalDestinationAggergationSwitchID);
    debug_print("Destination Aggregation Switch ID: s{}", destinationAggregationSwitchID);
    debug_print("Pod Local Destination ID: {}", podLocalDestinationHostID);
    debug_print("Pod Local Destination ToR Switch ID: {}", podLocalDestinationToRSwitchID);
    debug_print("Destination ToR Switch ID: s{}", destinationToRSwitchID);
#endif

    return route;
}



/**
 * Get the list of accessible switches for a trigger switch.
 *
 * @param triggerSwitchID   Switch ID for trigger switch
 *
 * @return validSwitches    Set of switches which are accessible from the trigger switch
 * 
 */
std::unordered_set<switch_id_t> getValidSwitches(switch_id_t triggerSwitchID, ft_scale_t fatTreeTopoK) {
    std::unordered_set<switch_id_t> validSwitches;

    const uint64_t k = fatTreeTopoK;
    const uint64_t kBy2 = fatTreeTopoK / 2;
    const uint64_t kSquare = k * k;
    const uint64_t kBy2WholeSquare = kBy2 * kBy2;

    if (triggerSwitchID < kSquare) {

        if (triggerSwitchID < kSquare) {
            if ((triggerSwitchID % k) < kBy2) {
                // ToR switch

                // add all ToR and Aggr switches in all the pods
                for (int i = 0; i < kSquare; i++) {
                    validSwitches.insert(i);
                }

                for (int i = kSquare; i < (kSquare +kBy2WholeSquare); i++) {
                    validSwitches.insert(i);
                }

                return validSwitches;
            } else {
                // Aggr switch
                switch_id_t podLocalAggregationSwitchID = (triggerSwitchID % k) - kBy2;
                switch_id_t indexForStartingCoreSwitchID = kSquare + (podLocalAggregationSwitchID * kBy2);

                // add all ToR switches
                for (int i = 0; i < kSquare; i++) {
                    if (i % k < kBy2) {
                        validSwitches.insert(i);
                    }
                }

                // add connected Core switches
                for (int i = 0; i < kBy2; i++) {
                    validSwitches.insert(indexForStartingCoreSwitchID + i);
                }

                // add connected Aggr switches
                for (int i = 0; i < k; i++) {
                    validSwitches.insert((k * i) + kBy2 + podLocalAggregationSwitchID);
                }

                return validSwitches;
            }
        }
    } else {
        // Core switch
        switch_id_t localgroupCoreSwitchID = (triggerSwitchID - kSquare) / kBy2;

        // add all ToR and connected Aggr switches
        for (int i = 0; i < kSquare; i++) {
            if (i % k < kBy2) {
                validSwitches.insert(i);
            }
            else if (i % kBy2 == localgroupCoreSwitchID) {
                validSwitches.insert(i);
            }
        }

        return validSwitches;
    }

    return validSwitches;
}


int main(int argc, char *argv[]) {

    ndebug_print_yellow("Welcome to data analysis!");

    int startTriggerID = 0, endTriggerID = 299;
    if (argc > 1) {
        startTriggerID = std::stoi(argv[1]);
        endTriggerID = std::stoi(argv[2]);
    }
    ndebug_print("Processing triggers from {} to {}.", startTriggerID, endTriggerID);

    Config syndbConfig;
    DataParser dataparser(PREFIX_FILE_PATH, PREFIX_STRING_FOR_DATA_FILES, syndbConfig.numSwitches);
    dataparser.getTriggerInfo(syndbConfig.numSwitches);

    pkt_id_t windowSize = WINDOW_SIZE;

    auto iteratorForTrigger = dataparser.listOfTriggers.begin();
    for (; iteratorForTrigger < dataparser.listOfTriggers.end(); iteratorForTrigger++) {

        switch_id_t triggerSwitchID = iteratorForTrigger->originSwitch;
        sim_time_t triggerTime = iteratorForTrigger->triggerTime;

        if (iteratorForTrigger->triggerId > endTriggerID || iteratorForTrigger->triggerId < startTriggerID) {
            continue;
        }

        ndebug_print_yellow("Trigger ID: {} Switch: {} Time: {}", iteratorForTrigger->triggerId, triggerSwitchID, triggerTime);

        // get p-record window for trigger switch
        std::map<pkt_id_t, PacketInfo> pRecordWindowForTriggerSwitch = dataparser.getWindowForSwitch(triggerSwitchID, triggerTime, windowSize, true);
        auto iteratorForpRecordWindowForTriggerSwitch = pRecordWindowForTriggerSwitch.begin();

        std::unordered_set<switch_id_t> validSwitches;
        if (syndbConfig.topoType == TopologyType::FatTree) {
            validSwitches = getValidSwitches(triggerSwitchID, syndbConfig.fatTreeTopoK);
        }
        ndebug_print_yellow("-----------------------");

#ifdef DEBUG
        for (; iteratorForpRecordWindowForTriggerSwitch != pRecordWindowForTriggerSwitch.end(); iteratorForpRecordWindowForTriggerSwitch++) {
            debug_print("{}\t{}\t{}\t{}", iteratorForpRecordWindowForTriggerSwitch->first,
                        iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime,
                        iteratorForpRecordWindowForTriggerSwitch->second.srcHost,
                        iteratorForpRecordWindowForTriggerSwitch->second.dstHost);
        }
#endif

        for (int switchID = 0; switchID < syndbConfig.numSwitches; switchID++) {
            
            std::string prefixFilePath = PREFIX_FILE_PATH;
            std::string prefixStringForFileName = PREFIX_STRING_FOR_DATA_FILES;
            std::string pathForDataFolder = prefixFilePath + "/" + prefixStringForFileName + "/" + prefixStringForFileName;
            std::string fileName = pathForDataFolder + "_switch_" + std::to_string(switchID) + ".txt";

            if (syndbConfig.topoType == TopologyType::FatTree) {
                auto isValidSwitch = validSwitches.find(switchID);
                if (isValidSwitch == validSwitches.end()) {
                    continue;
                }
            }

            if (switchID == triggerSwitchID) {
                continue;
            }

            // ----- Get precord window for switch when it receives the trigger packet -----
            sim_time_t timeForTriggerPacket = iteratorForTrigger->mapOfSwitchTriggerTime.find(switchID)->second;
            ndebug_print("\tSwitch: {}\t Trigger Packet Time: {}", switchID, timeForTriggerPacket);

            std::map<pkt_id_t, PacketInfo> pRecordWindowForCurrentSwitch = dataparser.getWindowForSwitch(switchID, timeForTriggerPacket, windowSize, false);

#ifdef DEBUG
            auto iteratorForpRecordWindowForCurrentSwitch = pRecordWindowForCurrentSwitch.begin();
            debug_print("-----------------------");
            for (; iteratorForpRecordWindowForCurrentSwitch != pRecordWindowForCurrentSwitch.end(); iteratorForpRecordWindowForCurrentSwitch++) {
                debug_print("{}\t{}", iteratorForpRecordWindowForCurrentSwitch->first,
                            iteratorForpRecordWindowForCurrentSwitch->second.switchIngressTime);
            }
#endif

            // ----- For each packet in the trigger switch precord window find the packet in current switch ID -----
            double numberOfExpectedPackets = 0.0, numberOfCommonPackets = 0.0;
            auto iteratorForpRecordWindowForTriggerSwitch = pRecordWindowForTriggerSwitch.begin();
            auto iteratorForpRecordWindowForCurrentSwitch = pRecordWindowForCurrentSwitch.begin();
            sim_time_t timeOfMostRecentCommonpRecord = 0, timeOfLeastRecentCommonpRecord = 0;
            pkt_id_t packetIDOfLeastRecentExpectedRecord, packetIDOfMostRecentCommonRecord;

            if (syndbConfig.topoType == TopologyType::FatTree) {
                for (; iteratorForpRecordWindowForTriggerSwitch != pRecordWindowForTriggerSwitch.end(); iteratorForpRecordWindowForTriggerSwitch++) {

                    auto route = getRoute(iteratorForpRecordWindowForTriggerSwitch->second.srcHost,
                                        iteratorForpRecordWindowForTriggerSwitch->second.dstHost,
                                        syndbConfig.fatTreeTopoK);

                    int indexOfTriggerSwitch = -1, indexOfCurrentSwitch = -1;
                    for (int routeIteratorIndex = 0; routeIteratorIndex < route.size(); routeIteratorIndex++) {
                        if (route[routeIteratorIndex] == triggerSwitchID) {
                            indexOfTriggerSwitch = routeIteratorIndex;
                        } else if (route[routeIteratorIndex] == switchID) {
                            indexOfCurrentSwitch = routeIteratorIndex;
                        }
                    }

                    if (indexOfCurrentSwitch != -1 && indexOfCurrentSwitch < indexOfTriggerSwitch)
                    {
                        numberOfExpectedPackets++;
                        
                        if (iteratorForpRecordWindowForCurrentSwitch == pRecordWindowForCurrentSwitch.end()) {
                            continue;
                        }

                        if (iteratorForpRecordWindowForTriggerSwitch->first < iteratorForpRecordWindowForCurrentSwitch->first) {
                            continue;
                        } 

                        while (iteratorForpRecordWindowForTriggerSwitch->first > iteratorForpRecordWindowForCurrentSwitch->first && 
                                iteratorForpRecordWindowForCurrentSwitch != pRecordWindowForCurrentSwitch.end()) {
                            ++iteratorForpRecordWindowForCurrentSwitch;
                        }
                        if (iteratorForpRecordWindowForTriggerSwitch->first == iteratorForpRecordWindowForCurrentSwitch->first) {
                            numberOfCommonPackets++;

                            if (iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime > timeOfMostRecentCommonpRecord) {
                                timeOfMostRecentCommonpRecord = iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime;
                                packetIDOfMostRecentCommonRecord = iteratorForpRecordWindowForTriggerSwitch->first;
                            }

                            if (iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime < timeOfLeastRecentCommonpRecord || timeOfLeastRecentCommonpRecord == 0) {
                                timeOfLeastRecentCommonpRecord = iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime;
                            }
                        }
                    }
                } 

                // ----- Calculate correlation and time lost statistics -----
                if (numberOfExpectedPackets != 0) {

                    if (timeOfLeastRecentCommonpRecord > timeOfMostRecentCommonpRecord && numberOfCommonPackets == 0) {
                        timeOfLeastRecentCommonpRecord = timeOfMostRecentCommonpRecord;
                    }

                    ndebug_print("\tSwitch ID: {}\t Expected Packets: {}\t Correlation: {}%\t Common History: {}ns",
                                switchID,
                                int(numberOfExpectedPackets),
                                100 * numberOfCommonPackets / numberOfExpectedPackets,
                                timeOfMostRecentCommonpRecord - timeOfLeastRecentCommonpRecord);
                }
            } else if (syndbConfig.topoType == TopologyType::Line) {

                for (; iteratorForpRecordWindowForTriggerSwitch != pRecordWindowForTriggerSwitch.end(); iteratorForpRecordWindowForTriggerSwitch++) {
                    auto isPacketInCurrentSwitchID = pRecordWindowForCurrentSwitch.find(iteratorForpRecordWindowForTriggerSwitch->first);
                    if (isPacketInCurrentSwitchID != pRecordWindowForCurrentSwitch.end()) {
                        numberOfCommonPackets++;

                        if (isPacketInCurrentSwitchID->second.switchIngressTime > timeOfMostRecentCommonpRecord) {
                            timeOfMostRecentCommonpRecord = isPacketInCurrentSwitchID->second.switchIngressTime;
                            packetIDOfMostRecentCommonRecord = isPacketInCurrentSwitchID->first;
                        }
                    }
                } // loop iterating over all packets in trigger switch window

                    
                float correlation = ((float)numberOfCommonPackets / (float)WINDOW_SIZE);
                ndebug_print("\tSwitch ID: {}\t Correlation: {}%\t ",
                                switchID,
                                correlation * 100);
            }
            

        } // loop iterating over all non-trigger switches

    } // loop iterating over all triggers

    return 0;
}

