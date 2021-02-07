#ifndef FTSWITCHES_H
#define FTSWITCHES_H

#include "topology/switch.hpp"


/* Abstract class for common implementations between ToR/Aggr switches */
struct SwitchFtTorAggr : Switch
{
    ft_scale_t fatTreeScaleK;
    pod_id_t podId;
    // For ToR: <racklocalhost_id, aggr switch_id>
    // For Aggr: <racklocalhost_id, core switch_id>
    std::unordered_map<racklocal_host_id_t, switch_id_t> routingTable; 

    /* Functions specific to ToR/Aggr switch's routing */
    Switch* getNextHop(host_id_t dstHostId);

    inline SwitchFtTorAggr(switch_id_t id) : Switch(id){
        this->fatTreeScaleK = syndbConfig.fatTreeTopoK;
    };
};


struct SwitchFtTor : SwitchFtTorAggr
{
    // resuse constructor of the base class
    using SwitchFtTorAggr::SwitchFtTorAggr;

    /* Overriding the virtual function of the base class */
    syndb_status_t routeScheduleNormalPkt(normalpkt_p &pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);
    
};

struct SwitchFtAggr : SwitchFtTorAggr
{
    std::vector<switch_id_t> coreSwitchesList;
    // resuse constructor of the base class
    using SwitchFtTorAggr::SwitchFtTorAggr;
    
    /* Overriding the virtual function of the base class */
    syndb_status_t routeScheduleNormalPkt(normalpkt_p &pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);

};


struct SwitchFtCore : Switch
{
    ft_scale_t fatTreeScaleK;
    // <pod_id, aggr_switch_id>
    std::unordered_map<pod_id_t, switch_id_t> routingTable;

    /* Overriding the virtual function of the base class */
    syndb_status_t routeScheduleNormalPkt(normalpkt_p &pkt, const sim_time_t pktArrivalTime, routeScheduleInfo &rsinfo);

    Switch* getNextHop(host_id_t dstHostId);

    inline SwitchFtCore(switch_id_t id) : Switch(id) {
        this->fatTreeScaleK = syndbConfig.fatTreeTopoK;
    };
};




#endif
