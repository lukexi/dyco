
#include "ringbuffer.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <memory.h>
void CreateRingBuffer(
    ringbuffer* RingBufferOut,
    ring_buffer_size_t ElementSizeBytes,
    ring_buffer_size_t ElementCount) {

    RingBufferOut->Storage = malloc(ElementSizeBytes * ElementCount);
    if (RingBufferOut->Storage == NULL) {
        printf("Ring buffer malloc of %i bytes failed\n", (int)(ElementSizeBytes * ElementCount));
        exit(1);
    }

    ring_buffer_size_t Result = PaUtil_InitializeRingBuffer(
        &RingBufferOut->RingBuffer,
        ElementSizeBytes,
        ElementCount,
        RingBufferOut->Storage);
    if (Result != 0) {
        printf("Ring buffer count not a power of 2 (%i)\n", (int)ElementCount);
        exit(1);
    }
}

ring_buffer_size_t GetRingBufferReadAvailable(ringbuffer* RingBuffer) {
    return PaUtil_GetRingBufferReadAvailable(&RingBuffer->RingBuffer);
}

ring_buffer_size_t GetRingBufferWriteAvailable(ringbuffer* RingBuffer) {
    return PaUtil_GetRingBufferWriteAvailable(&RingBuffer->RingBuffer);
}


ring_buffer_size_t ReadRingBuffer(
    ringbuffer*        RingBuffer,
    void*              Result,
    ring_buffer_size_t ElementCount)
{
    return PaUtil_ReadRingBuffer(
        &RingBuffer->RingBuffer,
        Result,
        ElementCount);
}

ring_buffer_size_t WriteRingBuffer(
    ringbuffer*        RingBuffer,
    const void*        Data,
    ring_buffer_size_t ElementCount)
{
    return PaUtil_WriteRingBuffer(
        &RingBuffer->RingBuffer,
        Data,
        ElementCount);
}



// Same as PaUtil_ReadRingBuffer, but doesn't advance read index
ring_buffer_size_t PeekRingBuffer(
    ringbuffer*        RingBuffer,
    void*              data,
    ring_buffer_size_t elementCount)
{
    PaUtilRingBuffer* rbuf = &RingBuffer->RingBuffer;
    ring_buffer_size_t size1, size2, numRead;
    void *data1, *data2;
    numRead = PaUtil_GetRingBufferReadRegions( rbuf, elementCount, &data1, &size1, &data2, &size2 );
    if( size2 > 0 )
    {
        memcpy( data, data1, size1*rbuf->elementSizeBytes );
        data = ((char *)data) + size1*rbuf->elementSizeBytes;
        memcpy( data, data2, size2*rbuf->elementSizeBytes );
    }
    else
    {
        memcpy( data, data1, size1*rbuf->elementSizeBytes );
    }
    return numRead;
}

ring_buffer_size_t AdvanceRingBufferReadIndex(
    ringbuffer* RingBuffer,
    ring_buffer_size_t elementCount)
{
    PaUtilRingBuffer* rbuf = &RingBuffer->RingBuffer;
    return PaUtil_AdvanceRingBufferReadIndex(rbuf, elementCount);
}


void FreeRingBuffer(ringbuffer* RingBuffer) {
    free(RingBuffer->Storage);
}

