#include "topology/switch_syndb.hpp"

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

pRecord RingBuffer::getPrecord(int32_t idx){
    return this->pRecordArray.at(idx); 
}

int32_t RingBuffer::getStart(){
    if(this->isEmpty())
        return -1;
    else if (this->wrapAround == false)
        return 0;
    else    
        return this->next;
}

int32_t RingBuffer::getEnd(){
    return this->end;
}

bool RingBuffer::isEmpty(){
    return this->end == -1; 
}

actualRingBufferInfo RingBuffer::getActualRingBufferInfo(uint32_t actualSize){
    actualRingBufferInfo info;
    int32_t start, end;
    int32_t elementsTillEnd, remainingElements;
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


/* 

Example code to iterate over ring buffer

int idx = s0->ringBuffer.getStart(); 
do
{
    ndebug_print("{}", s0->ringBuffer.getPrecord(idx).pktId);
    idx = (idx + 1) % syndbConfig.ringBufferSize;
} while (idx != s0->ringBuffer.getEnd()+1);

 */