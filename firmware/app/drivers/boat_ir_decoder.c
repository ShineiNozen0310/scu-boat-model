#include "boat_ir_decoder.h"

#include "../boat_config.h"

static bool within_tolerance(uint16_t value, uint16_t target, uint16_t tolerance)
{
    int32_t delta = (int32_t)value - (int32_t)target;
    return delta >= -(int32_t)tolerance && delta <= (int32_t)tolerance;
}

static void reset_collection(BoatIrDecoder *decoder)
{
    decoder->frame = 0U;
    decoder->bit_index = 0U;
    decoder->collecting = false;
}

bool BoatIrDecoder_FrameIsValid(uint32_t frame)
{
    uint8_t address = (uint8_t)(frame & 0xFFU);
    uint8_t address_inv = (uint8_t)((frame >> 8) & 0xFFU);
    uint8_t command = (uint8_t)((frame >> 16) & 0xFFU);
    uint8_t command_inv = (uint8_t)((frame >> 24) & 0xFFU);

    return address == (uint8_t)(~address_inv) &&
           command == (uint8_t)(~command_inv);
}

uint8_t BoatIrDecoder_Address(uint32_t frame)
{
    return (uint8_t)(frame & 0xFFU);
}

uint8_t BoatIrDecoder_Command(uint32_t frame)
{
    return (uint8_t)((frame >> 16) & 0xFFU);
}

void BoatIrDecoder_Init(BoatIrDecoder *decoder)
{
    decoder->frame = 0U;
    decoder->last_valid_frame = 0U;
    decoder->bit_index = 0U;
    decoder->collecting = false;
}

BoatIrDecodeResult BoatIrDecoder_PushIntervalUs(BoatIrDecoder *decoder, uint16_t interval_us, uint32_t *out_frame)
{
    uint8_t bit_value;
    uint32_t frame;

    if (out_frame != 0)
    {
        *out_frame = 0U;
    }

    if (within_tolerance(interval_us, BOAT_IR_START_US, BOAT_IR_START_TOLERANCE_US))
    {
        decoder->frame = 0U;
        decoder->bit_index = 0U;
        decoder->collecting = true;
        return BOAT_IR_RESULT_NONE;
    }

    if (within_tolerance(interval_us, BOAT_IR_REPEAT_US, BOAT_IR_REPEAT_TOLERANCE_US))
    {
        if (decoder->last_valid_frame != 0U)
        {
            if (out_frame != 0)
            {
                *out_frame = decoder->last_valid_frame;
            }
            return BOAT_IR_RESULT_REPEAT;
        }

        return BOAT_IR_RESULT_ERROR;
    }

    if (!decoder->collecting)
    {
        return BOAT_IR_RESULT_NONE;
    }

    if (within_tolerance(interval_us, BOAT_IR_BIT0_US, BOAT_IR_BIT_TOLERANCE_US))
    {
        bit_value = 0U;
    }
    else if (within_tolerance(interval_us, BOAT_IR_BIT1_US, BOAT_IR_BIT_TOLERANCE_US))
    {
        bit_value = 1U;
    }
    else
    {
        reset_collection(decoder);
        return BOAT_IR_RESULT_ERROR;
    }

    if (bit_value != 0U)
    {
        decoder->frame |= (1UL << decoder->bit_index);
    }

    decoder->bit_index++;
    if (decoder->bit_index < 32U)
    {
        return BOAT_IR_RESULT_NONE;
    }

    frame = decoder->frame;
    reset_collection(decoder);

    if (BoatIrDecoder_FrameIsValid(frame))
    {
        decoder->last_valid_frame = frame;
    }
    else
    {
        decoder->last_valid_frame = 0U;
    }

    if (out_frame != 0)
    {
        *out_frame = frame;
    }

    return BOAT_IR_RESULT_FRAME;
}
