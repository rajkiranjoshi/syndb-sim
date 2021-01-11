#include <stdio.h>
#include "utils/logger.hpp"

#include "traffic/packet.hpp"

int main(){

    normalPkt pkt; 
    triggerPkt tpkt;

    tpkt.triggerSwitchId = 99;

    printf("Size of struct NormalPacket is %ld bytes\n", sizeof(pkt));
    printf("Size of struct TriggerPacket is %ld bytes\n", sizeof(tpkt));

    log_debug("End of main\n");

    return 0;
}