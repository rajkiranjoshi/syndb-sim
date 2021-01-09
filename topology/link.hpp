#ifndef LINK_H
#define LINK_H

#include <cstdint>
#include <memory>

#include "utils/types.hpp"

/* Link struct */
typedef struct
{
    link_id_t id; 
    link_speed_gbps_t speed;
    time_t next_idle_time;
} Link;

typedef std::shared_ptr<Link> link_p;





#endif