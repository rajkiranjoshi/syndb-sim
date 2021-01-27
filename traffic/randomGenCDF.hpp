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

using namespace std;


typedef struct randomCDF
{
    vector<int> packetSizeDist;
    vector<int> flowArrivalDist;

    std::default_random_engine generator;
    std::default_random_engine generator2;

    std::uniform_int_distribution<int> uniformDist;

    std::lognormal_distribution<double> lognormalDist;

    vector<int> readCDFFile(string fileName);
    void loadCDFs(string packetsizeDistFile, string flowarrivalDistFile, int min_delay_ns);

    int getNextPacketSize();
    int getNextFlowDelay();
    int getNextPacketDelay();

} RandomFromCDF;

#endif
