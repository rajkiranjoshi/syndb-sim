#ifndef PACKET_H
#define PACKET_H

#include <cstdint>
#include <memory>
#include "utils/types.hpp"

/* Pkt Struct */
typedef struct Pkt {
    pkt_size_t size;

    Pkt() = default;
    Pkt(pkt_size_t size);

    ~Pkt();
    
} Pkt;

typedef struct NormalPkt : Pkt {
    pkt_id_t id;
    host_id_t srcHost;
    host_id_t dstHost;

    NormalPkt(pkt_id_t id, pkt_size_t size);
    // using Pkt::Pkt; // Inheriting constructor of base class Pkt

} NormalPkt;

typedef std::shared_ptr<NormalPkt> normalpkt_p;

/* triggerPkt Struct */
typedef struct TriggerPkt : Pkt {
    trigger_id_t triggerId;
    switch_id_t srcSwitchId;
    switch_id_t dstSwitchId;
    trigger_id_t triggerOriginSwId;
    sim_time_t triggerTime;

    TriggerPkt(trigger_id_t triggerId, pkt_size_t size);
    // using Pkt::Pkt; // Inheriting constructor of base class Pkt

} TriggerPkt;

typedef std::shared_ptr<TriggerPkt> triggerpkt_p;

enum class PacketType {NormalPkt, TriggerPkt};

#endif
