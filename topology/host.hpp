#ifndef HOST_H
#define HOST_H

#include <memory>
#include "utils/types.hpp"
#include "topology/link.hpp"
#include "topology/switch.hpp"
#include "traffic/trafficGenerator.hpp"

/* Host struct */
typedef struct
{
    host_id_t id;
    link_p torLink;
    switch_p torSwitch;

    normalpkt_p nextPkt;
    sim_time_t nextPktTime;

    trafficGenerator trafficGen;

} Host;

typedef std::shared_ptr<Host> host_p;




#endif