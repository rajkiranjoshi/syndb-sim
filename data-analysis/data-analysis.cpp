#include <iostream>
#include <vector>

#include "utils/logger.hpp"
#include "simulation/config.hpp"
#include "data-analysis/dataparser.hpp"

#define WINDOW_SIZE 1000000

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
    DataParser dataparser(PREFIX_FILE_PATH, PREFIX_STRING_FOR_DATA_FILES, syndbConfig.numSwitches, syndbConfig.numHosts);
    dataparser.getTriggerInfo(syndbConfig.numSwitches);

    pkt_id_t windowSize = WINDOW_SIZE;

    auto iteratorForTrigger = dataparser.listOfTriggers.begin();
    for (; iteratorForTrigger < dataparser.listOfTriggers.end(); iteratorForTrigger++) {

        
        switch_id_t triggerSwitchID = iteratorForTrigger->originSwitch;
        // switch_id_t triggerSwitchID = 0;
        sim_time_t triggerTime = iteratorForTrigger->triggerTime;

        if (iteratorForTrigger->triggerId != 1) {
            continue;
        }
        ndebug_print_yellow("Trigger ID: {} Switch: {} Time: {}", iteratorForTrigger->triggerId, triggerSwitchID, triggerTime);

        // get p-record window for trigger switch
        std::unordered_map<pkt_id_t, PacketInfo> pRecordWindowForTriggerSwitch = dataparser.getWindowForSwitch(triggerSwitchID, triggerTime, windowSize, true);

        auto iteratorForpRecordWindowForTriggerSwitch = pRecordWindowForTriggerSwitch.begin();
        ndebug_print_yellow("-----------------------");

#ifdef DEBUG
        for (; iteratorForpRecordWindowForTriggerSwitch != pRecordWindowForTriggerSwitch.end(); iteratorForpRecordWindowForTriggerSwitch++)
        {
            debug_print("{}\t{}\t{}\t{}", iteratorForpRecordWindowForTriggerSwitch->first,
                        iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime,
                        iteratorForpRecordWindowForTriggerSwitch->second.srcHost,
                        iteratorForpRecordWindowForTriggerSwitch->second.dstHost);
        }
#endif

        for (int switchID = 0; switchID < syndbConfig.numSwitches; switchID++)
        {
            // for (int switchID = 21; switchID < 24; switchID++) {
            std::string prefixFilePath = PREFIX_FILE_PATH;
            std::string prefixStringForFileName = PREFIX_STRING_FOR_DATA_FILES;
            std::string pathForDataFolder = prefixFilePath + "/dump_2_52_9/" + prefixStringForFileName;
            std::string fileName = pathForDataFolder + "switch_" + std::to_string(switchID) + ".txt";

            if (switchID == triggerSwitchID)
            {
                continue;
            }

            // ----- Get precord window for switch when it receives the trigger packet -----
            sim_time_t timeForTriggerPacket = iteratorForTrigger->mapOfSwitchTriggerTime.find(switchID)->second;
            // sim_time_t timeForTriggerPacket = triggerTime;
            debug_print("\tSwitch: {}\t Trigger Packet Time: {}", switchID, timeForTriggerPacket);
            std::unordered_map<pkt_id_t, PacketInfo> pRecordWindowForCurrentSwitch = dataparser.getWindowForSwitch(switchID, timeForTriggerPacket, windowSize, false);

#ifdef DEBUG
            auto iteratorForpRecordWindowForCurrentSwitch = pRecordWindowForCurrentSwitch.begin();
            debug_print("-----------------------");
            for (; iteratorForpRecordWindowForCurrentSwitch != pRecordWindowForCurrentSwitch.end(); iteratorForpRecordWindowForCurrentSwitch++)
            {
                debug_print("{}\t{}", iteratorForpRecordWindowForCurrentSwitch->first,
                            iteratorForpRecordWindowForCurrentSwitch->second.switchIngressTime);
            }
#endif

            // ----- For each packet in the trigger switch precord window find the packet in current switch ID -----
            double numberOfExpectedPackets = 0.0, numberOfCommonPackets = 0.0;
            auto iteratorForpRecordWindowForTriggerSwitch = pRecordWindowForTriggerSwitch.begin();
            sim_time_t timeOfMostRecentCommonpRecord = 0, timeOfLeastRecentCommonpRecord = 0;
            sim_time_t timeOfMostRecentExpectedpRecord = 0, timeOfLeastRecentExpectedpRecord = 0;
            pkt_id_t packetIDOfLeastRecentExpectedRecord, packetIDOfMostRecentCommonRecord;

            for (; iteratorForpRecordWindowForTriggerSwitch != pRecordWindowForTriggerSwitch.end(); iteratorForpRecordWindowForTriggerSwitch++)
            {

                auto route = getRoute(iteratorForpRecordWindowForTriggerSwitch->second.srcHost,
                                      iteratorForpRecordWindowForTriggerSwitch->second.dstHost,
                                      syndbConfig.fatTreeTopoK);

                int indexOfTriggerSwitch = -1, indexOfCurrentSwitch = -1;
                for (int routeIteratorIndex = 0; routeIteratorIndex < route.size(); routeIteratorIndex++)
                {
                    if (route[routeIteratorIndex] == triggerSwitchID)
                    {
                        indexOfTriggerSwitch = routeIteratorIndex;
                    }
                    else if (route[routeIteratorIndex] == switchID)
                    {
                        indexOfCurrentSwitch = routeIteratorIndex;
                    }
                }

                if (indexOfCurrentSwitch != -1 && indexOfCurrentSwitch < indexOfTriggerSwitch)
                {
                    numberOfExpectedPackets++;

                    if (iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime < timeOfLeastRecentExpectedpRecord || timeOfLeastRecentExpectedpRecord == 0)
                    {
                        timeOfLeastRecentExpectedpRecord = iteratorForpRecordWindowForTriggerSwitch->second.switchIngressTime;
                        packetIDOfLeastRecentExpectedRecord = iteratorForpRecordWindowForTriggerSwitch->first;
                    }

                    auto isPacketInCurrentSwitchID = pRecordWindowForCurrentSwitch.find(iteratorForpRecordWindowForTriggerSwitch->first);
                    if (isPacketInCurrentSwitchID != pRecordWindowForCurrentSwitch.end())
                    {
                        numberOfCommonPackets++;

                        if (isPacketInCurrentSwitchID->second.switchIngressTime > timeOfMostRecentCommonpRecord)
                        {
                            timeOfMostRecentCommonpRecord = isPacketInCurrentSwitchID->second.switchIngressTime;
                            packetIDOfMostRecentCommonRecord = isPacketInCurrentSwitchID->first;
                        }
                    }
                    /* 
                    else if (indexOfCurrentSwitch > indexOfTriggerSwitch){
                        debug_print("Expected packet: {}", iteratorForpRecordWindowForTriggerSwitch->first);

                        std::string getLineNumberForCurrentPacket = "cat " + fileName + "| cut -f 2 | grep -n -w " + std::to_string(iteratorForpRecordWindowForTriggerSwitch->first) + " | cut -d \":\" -f 1";
                        uint64_t lineNumber = std::stoll(dataparser.executeShellCommand(getLineNumberForCurrentPacket.c_str()));
                        std::string getSimulationTimeForCurrentPacket = "sed -n " + std::to_string(lineNumber) + "p " + fileName + " | cut -f 1";
                        uint64_t packetIngressTime = std::stoll(dataparser.executeShellCommand(getSimulationTimeForCurrentPacket.c_str()));

                        if (packetIngressTime > timeForTriggerPacket) {
                            numberOfExpectedPackets--;
                        }
                    }
                    */
                }
            } // loop iterating over all packets in trigger switch window

            // ----- Calculate correlation and time lost statistics -----
            if (numberOfExpectedPackets != 0)
            {
                uint64_t timeWhenPacketIDWasLastAvailable = timeForTriggerPacket;
                if (numberOfCommonPackets < numberOfExpectedPackets)
                {

                    std::string getLineNumber = "cat " + fileName + "| cut -f 2 | grep -n -w " + std::to_string(packetIDOfLeastRecentExpectedRecord) + " | cut -d \":\" -f 1";
                    uint64_t lineNumber = std::stoll(dataparser.executeShellCommand(getLineNumber.c_str()));

                    if (lineNumber < 1000000)
                    {
                        lineNumber = -1;
                    }
                    else
                    {
                        lineNumber -= 1000000;
                    }
                    debug_print("{}", lineNumber);

                    if (lineNumber < 100000000 && lineNumber > 0)
                    {
                        std::string getSimulationTime = "sed -n " + std::to_string(lineNumber) + "p " + fileName + " | cut -f 1";
                        timeWhenPacketIDWasLastAvailable = std::stoll(dataparser.executeShellCommand(getSimulationTime.c_str()));
                        debug_print("{}", timeWhenPacketIDWasLastAvailable);
                        if (timeWhenPacketIDWasLastAvailable > timeForTriggerPacket)
                        {
                            timeWhenPacketIDWasLastAvailable = timeForTriggerPacket;
                        }
                    }
                }

                ndebug_print("\tSwitch ID: {}\t Expected Packets: {}\t Correlation: {}%\t Time lost: {}ns",
                             switchID,
                             int(numberOfExpectedPackets),
                             100 * numberOfCommonPackets / numberOfExpectedPackets,
                             timeForTriggerPacket - timeWhenPacketIDWasLastAvailable);
            }

        } // loop iterating over all non-trigger switches

    } // loop iterating over all triggers

    return 0;
}



