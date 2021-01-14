#include <boost/log/trivial.hpp>
#include <fmt/color.h>
#include "utils/logger.hpp"

void log_debug(const std::string &msg){
    BOOST_LOG_TRIVIAL(debug) << msg;
}

void log_error(const std::string &msg){
    BOOST_LOG_TRIVIAL(error) << msg;
}

void log_info(const std::string &msg){
    BOOST_LOG_TRIVIAL(info) << msg;
}

void log_print_info(const std::string &msg){
    fmt::print(fg(fmt::color::yellow), msg);
}