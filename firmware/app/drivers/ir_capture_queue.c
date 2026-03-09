#include "ir_capture_queue.h"

static uint8_t next_index(uint8_t index)
{
    return (uint8_t)((index + 1U) % BOAT_IR_QUEUE_CAPACITY);
}

void BoatIrCaptureQueue_Init(BoatIrCaptureQueue *queue)
{
    queue->head = 0U;
    queue->tail = 0U;
    queue->overflowed = false;
}

bool BoatIrCaptureQueue_Push(BoatIrCaptureQueue *queue, uint16_t interval_us)
{
    uint8_t current_head = queue->head;
    uint8_t next_head = next_index(current_head);

    if (next_head == queue->tail)
    {
        queue->overflowed = true;
        return false;
    }

    queue->values[current_head] = interval_us;
    queue->head = next_head;
    return true;
}

bool BoatIrCaptureQueue_Pop(BoatIrCaptureQueue *queue, uint16_t *interval_us)
{
    uint8_t current_tail;

    if (queue->tail == queue->head)
    {
        return false;
    }

    current_tail = queue->tail;
    if (interval_us != 0)
    {
        *interval_us = queue->values[current_tail];
    }

    queue->tail = next_index(current_tail);
    return true;
}

bool BoatIrCaptureQueue_ConsumeOverflow(BoatIrCaptureQueue *queue)
{
    bool overflowed = queue->overflowed;
    queue->overflowed = false;
    return overflowed;
}
