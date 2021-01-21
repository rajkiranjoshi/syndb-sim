#include <iostream>
#include <fmt/core.h>
#include <fmt/color.h>
#include <boost/log/trivial.hpp>
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
