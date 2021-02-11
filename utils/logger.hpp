#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <stdarg.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include "simulation/simulation.hpp"


template <typename... Args>
void debug_print(const std::string &format, const Args&... args){
#ifdef DEBUG
    fmt::print(format, args...);
    std::putc('\n', stdout);
#endif
}

template <typename... Args>
void debug_print_yellow(const std::string &format, const Args&... args){
#ifdef DEBUG
    std::string msg = fmt::format(format, args...); 
    fmt::print(fg(fmt::color::yellow), msg);
    std::putc('\n', stdout);
#endif
}

template <typename... Args>
void ndebug_print(const std::string &format, const Args&... args){
    
    std::string msg = fmt::format(format, args...);

    #if LOGGING
        syndbSim.pktDumper->logSimSummary(msg);
    #endif

    fmt::print(msg);
    std::putc('\n', stdout);
}

template <typename... Args>
void ndebug_print_yellow(const std::string &format, const Args&... args){
    
    std::string msg = fmt::format(format, args...);

    #if LOGGING
        syndbSim.pktDumper->logSimSummary(msg);
    #endif
    
    fmt::print(fg(fmt::color::yellow), msg);
    std::putc('\n', stdout);
}





#endif
