#include "traffic/trafficPattern.hpp"

switch_id_t SimpleTopoTrafficPattern::applyTrafficPattern(switch_id_t srcSwitchId){
    // Hard-coded logic for destination host. Mainly for SimpleTopology
    return (srcSwitchId + 1) % 2; // 1 for 0 and 0 for 1
}

