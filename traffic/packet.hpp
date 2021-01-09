#ifndef PACKET_H
#define PACKET_H

#include <cstdint>
#include <memory>
#include "utils/types.hpp"

/* normalPkt Struct */
typedef struct {
    pkt_id_t id;
    pkt_size_t size;
    host_id_t srcHost;
    host_id_t dstHost;
} normalPkt;

typedef std::shared_ptr<normalPkt> normalpkt_p;

/* triggerPkt Struct */
typedef struct : normalPkt {
    switch_id_t triggerSwitchId;
} triggerPkt;

typedef std::shared_ptr<triggerPkt> triggerpkt_p;

#endif
