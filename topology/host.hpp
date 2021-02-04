#ifndef HOST_H
#define HOST_H

#include <memory>
#include "utils/types.hpp"
#include "topology/link.hpp"
#include "topology/switch.hpp"
#include "traffic/trafficGenerator.hpp"
#include "traffic/trafficPattern.hpp"

/* Host struct */
typedef struct Host
{
    host_id_t id;
    host_tor_link_p torLink;
    switch_p torSwitch;

    normalpkt_p nextPkt;
    sim_time_t nextPktTime;

    std::shared_ptr<TrafficGenerator> trafficGen;
    std::shared_ptr<TrafficPattern> trafficPattern;
    bool trafficGenDisabled;

    Host(host_id_t id, bool disableTrafficGen = false);

    void generateNextPkt();
    void sendPkt(normalpkt_p nextPkt, sim_time_t nextPktTime);

} Host;

typedef std::shared_ptr<Host> host_p;




#endif