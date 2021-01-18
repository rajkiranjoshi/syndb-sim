#include <fmt/core.h>
#include "simulation/event.hpp"
#include "simulation/simulation.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"

template<typename T>
PktEvent<T>::PktEvent(){
    // debug_print(fmt::format("New empty PktEvent constructed!"));
}

template<typename T>
PktEvent<T>::~PktEvent(){
    // debug_print(fmt::format("Event with packet id {} destructed!", this->pkt->id));
}

/* 
    To avoid linking error for the template's methods,
    instantiating the template class with the two possible types
*/

template class PktEvent<normalpkt_p>;
template class PktEvent<triggerpkt_p>;
