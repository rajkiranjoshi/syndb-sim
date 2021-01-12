#ifndef LINK_H
#define LINK_H

#include <cstdint>
#include <memory>

#include "utils/types.hpp"

/* Link struct */
typedef struct Link
{
    link_id_t id; 
    link_speed_gbps_t speed;
    time_t next_idle_time;

    Link();
    Link(link_id_t id, link_speed_gbps_t speed);
} Link;
/* Shared pointer to a Link struct object */
typedef std::shared_ptr<Link> link_p;





#endif