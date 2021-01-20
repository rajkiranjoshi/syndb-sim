#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

/* Simulation Related Types */
typedef uint64_t sim_time_t;

/* Packet related types */
typedef uint16_t pkt_size_t;
typedef uint64_t pkt_id_t;

/* Host related types */
typedef uint16_t host_id_t;

/* Switch related types */
typedef uint16_t switch_id_t;

/* Link Related Types */
typedef uint8_t link_speed_gbps_t;
typedef uint32_t link_id_t;

/* TrafficGen Related Types */
typedef uint8_t load_t;

/* SyNDB Related Types */
typedef int32_t ringbuffer_index_t;

typedef union {
    switch_id_t switch_id;
    host_id_t host_id;
} next_hop_id;

enum class syndb_status_t {success, failure};

#endif
