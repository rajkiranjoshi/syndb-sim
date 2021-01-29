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

void RandomFromCDF::loadCDFs (std::string packetsizeDistFile, std::string flowarrivalDistFile, int min_delay_ns) {
    packetSizeDist = readCDFFile(packetsizeDistFile);
    flowArrivalDist = readCDFFile(flowarrivalDistFile);
    uniformDist = std::uniform_int_distribution<>(0,99);
    lognormalDist = std::lognormal_distribution<>(min_delay_ns, min_delay_ns*10000);

}
    
int RandomFromCDF::getNextPacketSize () {
    int packetSize;
    int random = uniformDist(generator);
    packetSize = packetSizeDist.at(random);
    return packetSize;
}

sim_time_t RandomFromCDF::getNextFlowDelay () {
    sim_time_t flowDelay;
    int random = uniformDist(generator);
    flowDelay = flowArrivalDist.at(random);
    return flowDelay;
}

sim_time_t RandomFromCDF::getNextPacketDelay () {
    sim_time_t packet_delay;
    packet_delay = lognormalDist(generator2);
    return packet_delay;
}

