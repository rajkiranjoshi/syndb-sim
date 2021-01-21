#include "randomGenCDF.hpp"

vector<int> RandomFromCDF::readCDFFile(string fileName) {
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
    return distribution;
}   

void RandomFromCDF::loadCDFs(string packetsizeDistFile, string flowarrivalDistFile) {
    packetSizeDist = readCDFFile(packetsizeDistFile);
    flowArrivalDist = readCDFFile(flowarrivalDistFile);
    uniformDist = std::uniform_int_distribution<>(0,99);

}
    
int RandomFromCDF::getNextPacketSize() {
    int packetSize;
    int random = uniformDist(generator);
    packetSize = packetSizeDist.at(random);
    return packetSize;
}

int RandomFromCDF::getNextFlowDelay() {
    int flowDelay;
    int random = uniformDist(generator);
    flowDelay = flowArrivalDist.at(random);
    return flowDelay;
}

// int main () {
//     RandomFromCDF myRandomFromCDF;
//     myRandomFromCDF.loadCDFs("../traffic-dist/fb_webserver_packetsizedist_cdf.csv", "../traffic-dist/fb_webserver_flowinterarrival_ns_cdf.csv");
//     for (int i=0;i<10;i++) {
//         cout<<myRandomFromCDF.getNextPacketSize()<<"\n";
//     }

//     for (int i=0;i<10;i++) {
//         cout<<myRandomFromCDF.getNextFlowDelay()<<"\n";
//     }
//     return 0;
// }