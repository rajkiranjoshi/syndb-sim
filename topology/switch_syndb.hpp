#ifndef SWITCH_SYNDB_H
#define SWITCH_SYNDB_H

#include <array>
#include "utils/types.hpp"
#include "simulation/config.hpp"


struct pRecord
{
    pkt_id_t pktId;
    sim_time_t arrivalTime;
};

struct actualRingBufferInfo
{
    int32_t start;
    int32_t end;
};


struct RingBuffer
{
    private:

    std::array<pRecord, syndbConfig.ringBufferSize> pRecordArray;
    int32_t next, end;
    bool wrapAround;

    public:
    RingBuffer();
    void insertPrecord(pkt_id_t pktId, sim_time_t arrivalTime);
    pRecord getPrecord(int32_t idx);
    int32_t getStart();
    int32_t getEnd();
    actualRingBufferInfo getActualRingBufferInfo(uint32_t actualSize);
    bool isEmpty();
};


#endif