#ifndef PACKET_H
#define PACKET_H

#include <cstdint>
#include <memory>
#include "utils/types.hpp"

/* Pkt Struct */
typedef struct Pkt {
    pkt_id_t id;
    pkt_size_t size;

    Pkt() = default;
    Pkt(pkt_id_t id, pkt_size_t size);

    ~Pkt();
    
} Pkt;

typedef struct NormalPkt : Pkt {
    host_id_t srcHost;
    host_id_t dstHost;

    using Pkt::Pkt; // Inheriting constructor of base class Pkt

} NormalPkt;

typedef std::shared_ptr<NormalPkt> normalpkt_p;

/* triggerPkt Struct */
typedef struct : Pkt {
    switch_id_t srcSwitchId;
    switch_id_t dstSwitchId;

    using Pkt::Pkt; // Inheriting constructor of base class Pkt

} TriggerPkt;

typedef std::shared_ptr<TriggerPkt> triggerpkt_p;

#endif
