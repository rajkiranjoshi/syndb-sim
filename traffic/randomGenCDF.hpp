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
    
    std::uniform_int_distribution<int> uniformDist;

    vector<int> readCDFFile(string fileName);
    void loadCDFs(string packetsizeDistFile, string flowarrivalDistFile);

    int getNextPacketSize();
    int getNextFlowDelay();

} RandomFromCDF;

#endif
