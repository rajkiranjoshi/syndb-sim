/*
#include <cmath>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string.h>
#include <vector> 
#include <random>
#include <stdlib.h>


using namespace std;

typedef struct randomCDF
{
    std::vector<int> packetSizeDist;
    std::vector<int> packetArrivalDist;

    unsigned mySeed;

    std::default_random_engine generator;
    std::default_random_engine generator2;

    std::uniform_int_distribution<int> uniformDist;

    std::vector<int> readCDFFile(std::string fileName);
    void loadCDFs(std::string packetsizeDistFile, std::string flowarrivalDistFile, int min_delay_ns);

    int getNextPacketSize();
    uint64_t getNextFlowDelay();
    uint64_t getNextPacketDelay();

} RandomFromCDF;


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

void RandomFromCDF::loadCDFs (std::string packetsizeDistFile, std::string packetarrivalDistFile, int min_delay_ns) {
    packetSizeDist = readCDFFile(packetsizeDistFile);
    packetArrivalDist = readCDFFile(packetarrivalDistFile);
    mySeed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(mySeed);
    generator2.seed(mySeed);
    uniformDist = std::uniform_int_distribution<>(0,99);
    //lognormalDist = std::lognormal_distribution<>(min_delay_ns, min_delay_ns*10000);
    // lognormalDist = std::lognormal_distribution<>(5, 0.1);
    // weibullDist   = std::weibull_distribution<>(5, 0.1);
    // printf("Log min=%lf, mac=%lf\n", lognormalDist.min(), lognormalDist.max());
    // printf("Weibull min=%lf, mac=%lf\n", weibullDist.min(), weibullDist.max());

}
    
int RandomFromCDF::getNextPacketSize () {
    int packetSize;
    int random = uniformDist(generator);
    packetSize = packetSizeDist.at(random);
    return packetSize;
}


uint64_t RandomFromCDF::getNextPacketDelay () {
    uint32_t packetDelay;
    int random = uniformDist(generator2);
    packetDelay = packetArrivalDist.at(random);
    printf("%d\n", packetDelay);
    return packetDelay;
}

RandomFromCDF myRandomFromCDF;

uint32_t torLinkSpeed = 100;
uint32_t load = 100;


int loadTrafficDistribution (std::string packetsizeDistFile, std::string packetarrivalDistFile) {
    uint32_t base_size = 80; // in bytes
    uint32_t size_on_wire = base_size + 24;
    // int pps = ((this->torLinkSpeed * 1000000000)/ (size_on_wire * 8));
    // sim_time_t min_delay_ns = ((double)1/(double)pps * 1000000000);
    uint32_t min_delay_ns = (size_on_wire * 8) / torLinkSpeed;
    printf("Min delay = %u\n", min_delay_ns);
    myRandomFromCDF.loadCDFs(packetsizeDistFile, packetarrivalDistFile, min_delay_ns);
    return 0;
}

void getNextPacket(){
   
    uint32_t pkt_size = myRandomFromCDF.getNextPacketSize(); 
    uint64_t sendDelay = myRandomFromCDF.getNextPacketDelay();


    printf("Pkt Size = %d, sendDelay=%llu\n", pkt_size, sendDelay);

}


int main () {
    loadTrafficDistribution("../traffic-dist/fb_webserver_packetsizedist_cdf.csv", "../traffic-dist/packetinterarrival_ns_cdf.csv");
    for (int i=0;i<100;i++) {
        getNextPacket();
    }
    return 0;
}
*/