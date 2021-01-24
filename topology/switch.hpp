#ifndef SWITCH_H
#define SWITCH_H

#include <memory>
#include <unordered_map>
#include <set>
#include "utils/types.hpp"
#include "topology/link.hpp"
#include "topology/syndb_ringbuffer.hpp"
#include "traffic/packet.hpp"


typedef std::unordered_map<switch_id_t, switch_id_t> routing_table_t;
typedef std::unordered_map<switch_id_t, network_link_p> neighbor_switch_table_t;
typedef std::unordered_map<host_id_t, host_tor_link_p> neighbor_host_table_t;

enum class SwitchType {Simple, Rack, Aggregation, Core}; 

struct routeScheduleInfo;

struct Switch;
typedef std::shared_ptr<Switch> switch_p;


/* Switch struct */
struct Switch
{
    switch_id_t id;
    SwitchType type;
    sim_time_t hop_delay;
    neighbor_switch_table_t neighborSwitchTable;
    neighbor_host_table_t neighborHostTable;

    /* SyNDB specific members */
    RingBuffer ringBuffer;
    std::set<trigger_id_t> triggerHistory;

    /* Virtual functions to update routing */
    virtual void updateRouting(switch_id_t dstSwitchId, switch_id_t nextHopSwitchId) = 0;

    /* Switch's processing on receiving a pkt: logging, SyNDB, etc. */
    void receiveNormalPkt(normalpkt_p pkt, sim_time_t rxTime);
    void receiveTriggerPkt(triggerpkt_p pkt, sim_time_t rxTime);
    void generateTrigger();
    void createSendTriggerPkt(switch_id_t dstSwitchId, trigger_id_t triggerId, switch_id_t originSwitchId, sim_time_t origTriggerTime, sim_time_t pktArrivalTime);
    void snapshotRingBuffer();

    /* Fills rinfo. Returns Success or Failure */
    virtual syndb_status_t routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo) = 0;
    virtual syndb_status_t routeScheduleTriggerPkt(triggerpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo) = 0;
    
    syndb_status_t intraRackRouteNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    /* Inter-switch scheduling  */ 
    syndb_status_t scheduleToNextHopSwitch(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const switch_p nextHopSwitch, routeScheduleInfo &rsinfo, PacketType ptype);
    void schedulePkt(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const link_speed_gbps_t linkSpeed, sim_time_t &qNextIdleTime);

    
    Switch() = default; 
    Switch(switch_id_t id);

};

struct SimpleSwitch : Switch{

    routing_table_t routingTable;

    /* Overriding the virtual functions of the base class */
    void updateRouting(switch_id_t dstSwitchId, switch_id_t nextHopSwitchId);
    syndb_status_t routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    syndb_status_t routeScheduleTriggerPkt(triggerpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);

    switch_p getNextHop(switch_id_t dstSwitchId);

    SimpleSwitch(switch_id_t id):Switch(id) {};

};

struct routeScheduleInfo
{
    switch_p nextSwitch;
    sim_time_t pktNextForwardTime;
};

#endif
