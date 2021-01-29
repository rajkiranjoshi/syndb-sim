#ifndef RANDOMGENCDF_H
#define RANDOMGENCDF_H

#include <cmath>
#include <random>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "utils/types.hpp"


typedef struct randomCDF
{
    std::vector<int> packetSizeDist;
    std::vector<int> flowArrivalDist;

    std::default_random_engine generator;
    std::default_random_engine generator2;

    std::uniform_int_distribution<int> uniformDist;

    std::lognormal_distribution<double> lognormalDist;

    std::vector<int> readCDFFile(std::string fileName);
    void loadCDFs(std::string packetsizeDistFile, std::string flowarrivalDistFile, int min_delay_ns);

    int getNextPacketSize();
    sim_time_t getNextFlowDelay();
    sim_time_t getNextPacketDelay();

} RandomFromCDF;

#endif
