#ifndef IR_CAPTURE_QUEUE_H
#define IR_CAPTURE_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

#include "../boat_config.h"

typedef struct BoatIrCaptureQueue
{
    uint16_t values[BOAT_IR_QUEUE_CAPACITY];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile bool overflowed;
} BoatIrCaptureQueue;

void BoatIrCaptureQueue_Init(BoatIrCaptureQueue *queue);
bool BoatIrCaptureQueue_Push(BoatIrCaptureQueue *queue, uint16_t interval_us);
bool BoatIrCaptureQueue_Pop(BoatIrCaptureQueue *queue, uint16_t *interval_us);
bool BoatIrCaptureQueue_ConsumeOverflow(BoatIrCaptureQueue *queue);

#endif
