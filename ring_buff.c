/*
    Ring buffer with atomic operations
 */

#include "ring_buff.h"

void RingBuffer_Init(RingBuffer_t *buffer, uint8_t *buffer_ptr, uint16_t sz)
{
    buffer->size = sz;
    buffer->buffer = buffer_ptr;
    buffer->head = buffer->buffer;
    buffer->tail = buffer->buffer;
}

uint8_t RingBuffer_Push(RingBuffer_t *buffer, uint8_t data)
{
    *buffer->head = data;
    uint8_t *head = buffer->head + 1;
    if (head >= buffer->buffer + buffer->size) {
        head -= buffer->size;
    }
    buffer->head = head;
    return *buffer->head;
}

uint8_t RingBuffer_Pop(RingBuffer_t *buffer)
{
    uint8_t byte = *buffer->tail;

    uint8_t *tail = buffer->tail + 1;
    if (tail >= buffer->buffer + buffer->size) {
        tail -= buffer->size;
    }
    buffer->tail = tail;
    return byte;
}

uint16_t RingBuffer_Available(RingBuffer_t *buffer)
{
    uint8_t *tail = buffer->tail;
    uint8_t *head = buffer->head;
    uint16_t size = buffer->size;

    if (tail == head) {
        return 0;
    } else if (tail < head) {
        return head - tail;
    } else {
        return size - (tail - head);
    }
}

uint8_t RingBuffer_Peek(RingBuffer_t *buffer)
{
    return *buffer->tail;
}

void RingBuffer_Clear(RingBuffer_t *buffer)
{
    buffer->head = buffer->tail = buffer->buffer;
}
