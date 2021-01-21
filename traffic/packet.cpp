#include <fmt/core.h>
#include "traffic/packet.hpp"
#include "utils/logger.hpp"
#include "simulation/simulation.hpp"

Pkt::Pkt(pkt_size_t size){
    this->size = size;

    // debug_print(fmt::format("[Sim Time {}ns] Packet {} constructed!", syndbSim.currTime, id));
}

Pkt::~Pkt(){
    // debug_print(fmt::format("[Sim Time {}ns] Packet {} destructed!", syndbSim.currTime, this->id));
}


NormalPkt::NormalPkt(pkt_id_t id, pkt_size_t size):Pkt::Pkt(size){
    this->id = id;
}

TriggerPkt::TriggerPkt(trigger_id_t triggerId, pkt_size_t size):Pkt::Pkt(size){
    this->triggerId = triggerId;
}
