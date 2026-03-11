#include "boat_radio_protocol.h"

void BoatRadioFrame_InitNeutral(BoatRadioFrame *frame)
{
    if (frame != 0)
    {
        BoatCommand_InitNeutral(&frame->command);
        frame->sequence = 0U;
    }
}

uint8_t BoatRadioProtocol_Checksum(const uint8_t *payload, uint8_t length_without_checksum)
{
    uint8_t checksum = 0U;
    uint8_t index;

    if (payload == 0)
    {
        return 0U;
    }

    /* CRC-8/ATM over the fixed header and control fields. */
    for (index = 0U; index < length_without_checksum; ++index)
    {
        uint8_t bit_index;

        checksum ^= payload[index];
        for (bit_index = 0U; bit_index < 8U; ++bit_index)
        {
            if ((checksum & 0x80U) != 0U)
            {
                checksum = (uint8_t)((checksum << 1U) ^ 0x07U);
            }
            else
            {
                checksum <<= 1U;
            }
        }
    }

    return checksum;
}

bool BoatRadioProtocol_Encode(const BoatRadioFrame *frame, uint8_t *payload, uint8_t length)
{
    if (frame == 0 || payload == 0 || length < BOAT_RADIO_FRAME_LENGTH)
    {
        return false;
    }

    payload[0] = BOAT_RADIO_FRAME_SYNC;
    payload[1] = frame->sequence;
    payload[2] = (uint8_t)frame->command.throttle_percent;
    payload[3] = (uint8_t)frame->command.rudder_percent;
    payload[4] = (uint8_t)frame->command.turret_yaw_percent;
    payload[5] = (uint8_t)frame->command.turret_pitch_percent;
    payload[6] = frame->command.flags;
    payload[7] = BoatRadioProtocol_Checksum(payload, BOAT_RADIO_FRAME_PAYLOAD_LENGTH);
    return true;
}

BoatRadioDecodeStatus BoatRadioProtocol_Decode(
    const uint8_t *payload,
    uint8_t length,
    BoatRadioFrame *out_frame)
{
    if (payload == 0 || out_frame == 0)
    {
        return BOAT_RADIO_DECODE_INVALID_ARG;
    }

    if (length != BOAT_RADIO_FRAME_LENGTH)
    {
        return BOAT_RADIO_DECODE_INVALID_LENGTH;
    }

    if (payload[0] != BOAT_RADIO_FRAME_SYNC)
    {
        return BOAT_RADIO_DECODE_INVALID_SYNC;
    }

    if (payload[BOAT_RADIO_FRAME_LENGTH - 1U] !=
        BoatRadioProtocol_Checksum(payload, BOAT_RADIO_FRAME_PAYLOAD_LENGTH))
    {
        return BOAT_RADIO_DECODE_INVALID_CHECKSUM;
    }

    out_frame->sequence = payload[1];
    out_frame->command.throttle_percent = (int8_t)payload[2];
    out_frame->command.rudder_percent = (int8_t)payload[3];
    out_frame->command.turret_yaw_percent = (int8_t)payload[4];
    out_frame->command.turret_pitch_percent = (int8_t)payload[5];
    out_frame->command.flags = payload[6];
    return BOAT_RADIO_DECODE_OK;
}
