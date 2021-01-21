#ifndef LINK_H
#define LINK_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "utils/types.hpp"

/* Link struct */
typedef struct Link
{
    link_id_t id; 
    link_speed_gbps_t speed;

    Link() = default;
    Link(link_id_t id, link_speed_gbps_t speed);
} Link;

/* NetworkLink struct */
typedef struct NetworkLink : Link
{
    std::unordered_map<sim_time_t, sim_time_t> next_idle_time;
    std::unordered_map<sim_time_t, sim_time_t> next_idle_time_priority;

    NetworkLink(link_id_t id, link_speed_gbps_t speed, switch_id_t sw1, switch_id_t sw2);

} NetworkLink;

typedef struct HostTorLink : Link
{
    sim_time_t next_idle_time_to_tor;
    sim_time_t next_idle_time_to_host;

    HostTorLink(link_id_t id, link_speed_gbps_t speed);
} HostTorLink;

/* Shared pointer to a Link struct object */
typedef std::shared_ptr<NetworkLink> network_link_p;
typedef std::shared_ptr<HostTorLink> host_tor_link_p;





#endif