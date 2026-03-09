#ifndef BOAT_IR_DECODER_H
#define BOAT_IR_DECODER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum BoatIrDecodeResult
{
    BOAT_IR_RESULT_NONE = 0,
    BOAT_IR_RESULT_FRAME,
    BOAT_IR_RESULT_REPEAT,
    BOAT_IR_RESULT_ERROR
} BoatIrDecodeResult;

typedef struct BoatIrDecoder
{
    uint32_t frame;
    uint32_t last_valid_frame;
    uint8_t bit_index;
    bool collecting;
} BoatIrDecoder;

void BoatIrDecoder_Init(BoatIrDecoder *decoder);
BoatIrDecodeResult BoatIrDecoder_PushIntervalUs(BoatIrDecoder *decoder, uint16_t interval_us, uint32_t *out_frame);
bool BoatIrDecoder_FrameIsValid(uint32_t frame);
uint8_t BoatIrDecoder_Address(uint32_t frame);
uint8_t BoatIrDecoder_Command(uint32_t frame);

#endif
