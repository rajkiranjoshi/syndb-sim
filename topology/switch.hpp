#ifndef SWITCH_H
#define SWITCH_H

#include <memory>
#include <unordered_map>
#include "utils/types.hpp"
#include "topology/link.hpp"
#include "traffic/packet.hpp"


typedef std::unordered_map<switch_id_t, switch_id_t> routing_table_t;
typedef std::pair<switch_id_t, switch_id_t> routing_table_pair;
typedef std::unordered_map<switch_id_t, network_link_p> neighbor_switch_table_t;
typedef std::pair<switch_id_t, network_link_p> neighbor_switch_table_pair; 
typedef std::unordered_map<host_id_t, host_tor_link_p> neighbor_host_table_t;
typedef std::pair<host_id_t, host_tor_link_p> neighbor_host_table_pair;


struct routeScheduleInfo;
typedef struct routeScheduleInfo routeScheduleInfo;

struct Switch;
typedef struct Switch Switch;
typedef std::shared_ptr<Switch> switch_p;

/* Switch struct */
struct Switch
{
    switch_id_t id;
    sim_time_t hop_delay;
    neighbor_switch_table_t neighborSwitchTable;
    neighbor_host_table_t neighborHostTable;
    routing_table_t routingTable;

    /* Switch's processing on receiving a pkt: logging, SyNDB, etc. */
    void receiveNormalPkt(normalpkt_p pkt);
    void receiveTriggerPkt(triggerpkt_p pkt);

    /* Fills rinfo. Returns Success or Failure */
    status_t routeScheduleNormalPkt(normalpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    status_t routeScheduleTriggerPkt(triggerpkt_p pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    
    switch_p getNextHop(switch_id_t dstSwitchId);
    void schedulePkt(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const link_speed_gbps_t linkSpeed, sim_time_t &qNextIdleTime);
    /* Inter-switch routing+scheduling  */
    status_t routeScheduleToDstSwitch(const pkt_size_t pktsize, const sim_time_t pktArrivalTime, const switch_id_t dstSwitchId, routeScheduleInfo &rsinfo, PacketType ptype); 

    Switch() = default; 
    Switch(switch_id_t id);

};


struct routeScheduleInfo
{
    switch_p nextSwitch;
    sim_time_t pktNextForwardTime;
};



#endif
