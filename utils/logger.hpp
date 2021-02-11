#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <stdarg.h>
#include <fmt/core.h>
#include <fmt/color.h>


void log_debug(const std::string &msg);
void log_error(const std::string &msg);
void log_info(const std::string &msg);


template <typename... Args>
void debug_print(const std::string &format, const Args&... args){
#ifdef DEBUG_LOGGING
    fmt::print(format, args...);
    std::putc('\n', stdout);
#endif
}

template <typename... Args>
void debug_print_yellow(const std::string &format, const Args&... args){
#ifdef DEBUG_LOGGING
    std::string msg = fmt::format(format, args...); 
    fmt::print(fg(fmt::color::yellow), msg);
    std::putc('\n', stdout);
#endif
}

template <typename... Args>
void ndebug_print(const std::string &format, const Args&... args){
    fmt::print(format, args...);
    std::putc('\n', stdout);
}

template <typename... Args>
void ndebug_print_yellow(const std::string &format, const Args&... args){
    std::string msg = fmt::format(format, args...); 
    fmt::print(fg(fmt::color::yellow), msg);
    std::putc('\n', stdout);
}





#endif
