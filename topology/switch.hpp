#ifndef SWITCH_H
#define SWITCH_H

#include <memory>
#include <random>
#include <unordered_map>
#include <set>
#include <functional>
#include <fmt/core.h>
#include "utils/types.hpp"
#include "topology/link.hpp"
#include "topology/syndb_ringbuffer.hpp"
#include "traffic/packet.hpp"


typedef std::unordered_map<switch_id_t, network_link_p> neighbor_switch_table_t;
typedef std::unordered_map<host_id_t, host_tor_link_p> neighbor_host_table_t;

enum class SwitchType {Simple, FtTor, FtAggr, FtCore};

inline std::string switchTypeToString(SwitchType type){
    switch(type){
        case SwitchType::Simple:
            return "Simple";
            break;
        case SwitchType::FtTor:
            return "Tor";
            break;
        case SwitchType::FtAggr:
            return "Aggr";
            break; 
        case SwitchType::FtCore:
            return "Core";
            break;
        default:
            std::string msg = fmt::format("Unknown SwitchType {}", type);
            throw std::logic_error(msg);
    }
}

struct routeScheduleInfo;

struct Switch;
typedef std::shared_ptr<Switch> switch_p;


/* Switch struct */
struct Switch
{
    switch_id_t id;
    SwitchType type;
    sim_time_t hop_delay;
    sim_time_t hop_delay_variation;
    neighbor_switch_table_t neighborSwitchTable;
    neighbor_host_table_t neighborHostTable;
    std::default_random_engine randHopDelay;

    /* SyNDB specific members */
    RingBuffer ringBuffer;
    std::set<trigger_id_t> triggerHistory;

    /* Switch's processing on receiving a pkt: logging, SyNDB, etc. */
    void receiveNormalPkt(normalpkt_p pkt, sim_time_t rxTime);
    void receiveTriggerPkt(triggerpkt_p pkt, sim_time_t rxTime);
    void generateTrigger();
    void createSendTriggerPkt(switch_id_t dstSwitchId, trigger_id_t triggerId, switch_id_t originSwitchId, sim_time_t origTriggerTime, sim_time_t pktArrivalTime);
    void snapshotRingBuffer(sim_time_t triggerPktRcvTime);

    /* Fills rinfo. Returns Success or Failure */
    syndb_status_t routeScheduleTriggerPkt(triggerpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    virtual syndb_status_t routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo) = 0; // <--- pure virtual. Impl by derived classes
    
    syndb_status_t intraRackRouteNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    /* Inter-switch scheduling  */ 
    syndb_status_t scheduleToNextHopSwitch(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const switch_p nextHopSwitch, routeScheduleInfo &rsinfo, PacketType ptype);
    void schedulePkt(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const link_speed_gbps_t linkSpeed, sim_time_t &qNextIdleTime, byte_count_t &byte_count);

    sim_time_t getRandomHopDelay();
    
    Switch() = default; 
    Switch(switch_id_t id);

};

struct SimpleSwitch : Switch{

    std::unordered_map<switch_id_t, switch_id_t> routingTable;

    /* Overriding the virtual functions of the base class */
    syndb_status_t routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    

    switch_p getNextHop(switch_id_t dstSwitchId);

    SimpleSwitch(switch_id_t id):Switch(id) {};

};

struct routeScheduleInfo
{
    switch_p nextSwitch;
    sim_time_t pktNextForwardTime;
};

#endif
