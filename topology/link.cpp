#include <simulation/simulation.hpp>
#include <simulation/config.hpp>

Link::Link(link_id_t id, link_speed_gbps_t speed) {
    this->id = id;
    this->speed = speed;
}


NetworkLink::NetworkLink(link_id_t id, link_speed_gbps_t speed, switch_id_t sw1, switch_id_t sw2) : Link::Link(id, speed){

    this->next_idle_time[sw1] = 0;
    this->next_idle_time[sw2] = 0;

}


HostTorLink::HostTorLink(link_id_t id, link_speed_gbps_t speed) : Link::Link(id, speed){
    this->next_idle_time_to_tor = 0;
    this->next_idle_time_to_host = 0;
}
