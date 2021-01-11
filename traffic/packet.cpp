#include "traffic/packet.hpp"

normalPkt::normalPkt(pkt_id_t id, pkt_size_t size){
    this->id = id;
    this->size = size;
}