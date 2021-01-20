#include "topology/switch_syndb.hpp"
#include "utils/logger.hpp"

RingBuffer::RingBuffer(){
    this->end = -1; // ring buffer is empty
    this->next = 0; // points to the start after wrap around
    this->wrapAround = false;
}

void RingBuffer::insertPrecord(pkt_id_t pktId, sim_time_t arrivalTime){
    
    pRecordArray[next].pktId = pktId;
    pRecordArray[next].arrivalTime = arrivalTime;

    // Common pointers management
    this->end = this->next;
    this->next = (this->next + 1) % this->pRecordArray.size();

    if(this->next == 0 && this->wrapAround == false){ // next becomes zero after incr
        this->wrapAround = true;
    }
}

pRecord RingBuffer::getPrecord(ringbuffer_index_t idx){
    return this->pRecordArray.at(idx); 
}

ringbuffer_index_t RingBuffer::getStart(){
    if(this->isEmpty())
        return -1;
    else if (this->wrapAround == false)
        return 0;
    else    
        return this->next;
}

ringbuffer_index_t RingBuffer::getEnd(){
    return this->end;
}

bool RingBuffer::isEmpty(){
    return this->end == -1; 
}

actualRingBufferInfo RingBuffer::getActualRingBufferInfo(uint32_t actualSize){
    actualRingBufferInfo info;
    ringbuffer_index_t start, end;
    ringbuffer_index_t elementsTillEnd, remainingElements;
    end = this->getEnd();

    elementsTillEnd = end + 1; // since array is zero indexed

    if(elementsTillEnd < actualSize){ // either wrapped around OR not enough elements even for actualSize

        if(this->wrapAround == true){
            remainingElements = actualSize - elementsTillEnd;
            start = this->pRecordArray.size() - remainingElements;
        }
        else // NOT wrapped around and elements in ringBuffer < actualSize
        {
            start = 0;
        }
        
    }
    else // elementsTillEnd >= actualSize
    {
        start = (end + 1) - actualSize;
    }

    info.start = start;
    info.end = end;

    return info;
    
}

ringbuffer_index_t RingBuffer::getNextIndex(ringbuffer_index_t idx){
    return (idx + 1) % this->pRecordArray.size();
}

void RingBuffer::printRingBufferRange(ringbuffer_index_t start, ringbuffer_index_t end){
    pRecord precord;
    ringbuffer_index_t idx = start;
    ringbuffer_index_t terminating_idx = this->getNextIndex(end); // the idx after end
    do
    {
        precord = this->getPrecord(idx);
        ndebug_print("{} {}", precord.pktId, precord.arrivalTime);
        idx = this->getNextIndex(idx);
    } while (idx != terminating_idx);
}

void RingBuffer::printRingBuffer(){
    ringbuffer_index_t start, end;
    start = this->getStart();
    end = this->getEnd();

    this->printRingBufferRange(start, end); 

}


void RingBuffer::printActualRingBuffer(uint32_t actualSize){

    actualRingBufferInfo info = this->getActualRingBufferInfo(actualSize);

    this->printRingBufferRange(info.start, info.end);

}
