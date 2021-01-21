#ifndef UTILS_H
#define UTILS_H

#include "utils/types.hpp"

inline sim_time_t getSerializationDelay(pkt_size_t pktSize, link_speed_gbps_t linkSpeed){
    pkt_size_t size_on_wire = pktSize + 24;
    return ((size_on_wire * 8) / linkSpeed);
}


#endif
