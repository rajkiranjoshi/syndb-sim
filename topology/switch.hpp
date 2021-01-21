#ifndef SWITCH_H
#define SWITCH_H

#include <memory>
#include <unordered_map>
#include <set>
#include "utils/types.hpp"
#include "topology/link.hpp"
#include "topology/switch_syndb.hpp"
#include "traffic/packet.hpp"


typedef std::unordered_map<switch_id_t, switch_id_t> routing_table_t;
typedef std::unordered_map<switch_id_t, network_link_p> neighbor_switch_table_t;
typedef std::unordered_map<host_id_t, host_tor_link_p> neighbor_host_table_t;


struct routeScheduleInfo;

struct Switch;
typedef std::shared_ptr<Switch> switch_p;

/* Switch struct */
struct Switch
{
    switch_id_t id;
    sim_time_t hop_delay;
    neighbor_switch_table_t neighborSwitchTable;
    neighbor_host_table_t neighborHostTable;
    routing_table_t routingTable;

    /* SyNDB specific members */
    RingBuffer ringBuffer;
    std::set<trigger_id_t> triggerHistory;

    /* Switch's processing on receiving a pkt: logging, SyNDB, etc. */
    void receiveNormalPkt(normalpkt_p pkt, sim_time_t rxTime);
    void receiveTriggerPkt(triggerpkt_p pkt, sim_time_t rxTime);
    void generateTrigger();
    void createSendTriggerPkt(switch_id_t dstSwitchId, trigger_id_t triggerId, switch_id_t originSwitchId, sim_time_t origTriggerTime, sim_time_t pktArrivalTime);
    void snapshotRingBuffer();

    /* Fills rinfo. Returns Success or Failure */
    syndb_status_t routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    syndb_status_t routeScheduleTriggerPkt(triggerpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    
    switch_p getNextHop(switch_id_t dstSwitchId);
    void schedulePkt(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const link_speed_gbps_t linkSpeed, sim_time_t &qNextIdleTime);
    /* Inter-switch routing+scheduling  */
    syndb_status_t routeScheduleToDstSwitch(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const switch_id_t dstSwitchId, routeScheduleInfo &rsinfo, PacketType ptype); 

    Switch() = default; 
    Switch(switch_id_t id);

};


struct routeScheduleInfo
{
    switch_p nextSwitch;
    sim_time_t pktNextForwardTime;
};

#endif
