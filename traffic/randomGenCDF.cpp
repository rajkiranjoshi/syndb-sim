#include <chrono>
#include <thread>
#include "simulation/config.hpp"
#include "randomGenCDF.hpp"
using namespace std;


std::vector<int> RandomFromCDF::readCDFFile (string fileName) {
    fstream myCDFFile;
    string line;
    vector<pair<int, int> > cdfMatrix;
    vector<int> distribution;
    myCDFFile.open(fileName, ios::in);
    if (!myCDFFile) {
         printf("CDF file not found!!\n");
    }
    while (getline(myCDFFile, line)) {
        pair<int, int> myPair;
        std::istringstream iss(line);
        int a, b;
        char c;
        iss>>a;
        distribution.push_back(a);
    }
    myCDFFile.close();
    return distribution;
}   

void RandomFromCDF::loadCDFs (std::string packetsizeDistFile, std::string packetarrivalDistFile) {
    packetSizeDist = readCDFFile(packetsizeDistFile);
    packetArrivalDist = readCDFFile(packetarrivalDistFile);
    uint64_t mySeed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    uint64_t mySeed2 = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(mySeed1);
    generator2.seed(mySeed2);
    uniformDist = std::uniform_int_distribution<>(0,99);
}
    
int RandomFromCDF::getNextPacketSize () {
    int packetSize;
    int random = uniformDist(generator);
    packetSize = packetSizeDist.at(random);
    return packetSize;
}

int RandomFromCDF::getNextPacketDelay () {
    int packetDelay;
    int random = uniformDist(generator2);
    packetDelay = packetArrivalDist.at(random);
    packetDelay = (int)((double) packetDelay / ((double)syndbConfig.targetBaseNetworkLoadPercent / 3.0));
    return packetDelay;
}

