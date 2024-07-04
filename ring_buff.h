/*
 ============================================================================
 Name        : ring_buff.h
 Author      :
 Version     : 
 Copyright   : Your copyright notice
 Description : code description
 ============================================================================
 */

#ifndef INC_RING_BUFF_H_
#define INC_RING_BUFF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    uint8_t* buffer;
    uint8_t* head;  // Points to the next byte to be written
    uint8_t* tail;  // Points to the next byte to be read
    uint16_t size;
} RingBuffer_t;

void RingBuffer_Init(RingBuffer_t *buffer, uint8_t *buffer_ptr, uint16_t sz);
int8_t RingBuffer_Push(RingBuffer_t *buffer, uint8_t data);
uint8_t RingBuffer_Pop(RingBuffer_t *buffer);
uint16_t RingBuffer_Available(RingBuffer_t *buffer);
uint8_t RingBuffer_Peek(RingBuffer_t *buffer);
void RingBuffer_Clear(RingBuffer_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /* INC_RING_BUFF_H_ */
