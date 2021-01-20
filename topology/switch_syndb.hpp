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
    ringbuffer_index_t start;
    ringbuffer_index_t end;
};


struct RingBuffer
{
    private:

    std::array<pRecord, syndbConfig.ringBufferSize> pRecordArray;
    ringbuffer_index_t next, end;
    bool wrapAround;

    public:
    RingBuffer();
    void insertPrecord(pkt_id_t pktId, sim_time_t arrivalTime);
    pRecord getPrecord(ringbuffer_index_t idx);
    ringbuffer_index_t getStart();
    ringbuffer_index_t getEnd();
    actualRingBufferInfo getActualRingBufferInfo(uint32_t actualSize);
    bool isEmpty();

    void printRingBufferRange(ringbuffer_index_t start, ringbuffer_index_t end);
    void printRingBuffer();
    void printActualRingBuffer(uint32_t actualSize);
    ringbuffer_index_t getNextIndex(ringbuffer_index_t idx); 
};


#endif
