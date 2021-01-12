#include <simulation/simulation.hpp>
#include <simulation/config.hpp>

Link::Link(){
    this->next_idle_time=0;
}

Link::Link(link_id_t id, link_speed_gbps_t speed) : Link::Link(){
    this->id = id;
    this->speed = speed;
}