#include "boat_radio_protocol.h"

static void boat_radio_parser_reset(BoatRadioParser *parser)
{
    if (parser != 0)
    {
        parser->index = 0U;
    }
}

static void boat_radio_parser_seek_next_sync(BoatRadioParser *parser)
{
    uint8_t next_sync_index;

    if (parser == 0)
    {
        return;
    }

    for (next_sync_index = 1U; next_sync_index < parser->index; ++next_sync_index)
    {
        if (parser->buffer[next_sync_index] == BOAT_RADIO_FRAME_SYNC)
        {
            uint8_t remaining = (uint8_t)(parser->index - next_sync_index);
            uint8_t copy_index;

            for (copy_index = 0U; copy_index < remaining; ++copy_index)
            {
                parser->buffer[copy_index] = parser->buffer[next_sync_index + copy_index];
            }

            parser->index = remaining;
            return;
        }
    }

    boat_radio_parser_reset(parser);
}

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

void BoatRadioParser_Init(BoatRadioParser *parser)
{
    boat_radio_parser_reset(parser);
}

BoatRadioParseResult BoatRadioParser_PushByte(
    BoatRadioParser *parser,
    uint8_t byte,
    BoatRadioFrame *out_frame)
{
    if (parser == 0)
    {
        return BOAT_RADIO_PARSE_ERROR;
    }

    if (parser->index == 0U)
    {
        if (byte != BOAT_RADIO_FRAME_SYNC)
        {
            return BOAT_RADIO_PARSE_NONE;
        }

        parser->buffer[parser->index++] = byte;
        return BOAT_RADIO_PARSE_NONE;
    }

    if (parser->index >= BOAT_RADIO_FRAME_LENGTH)
    {
        boat_radio_parser_reset(parser);
        return BOAT_RADIO_PARSE_ERROR;
    }

    parser->buffer[parser->index++] = byte;

    if (parser->index < BOAT_RADIO_FRAME_LENGTH)
    {
        return BOAT_RADIO_PARSE_NONE;
    }

    if (BoatRadioProtocol_Decode(parser->buffer, BOAT_RADIO_FRAME_LENGTH, out_frame) ==
        BOAT_RADIO_DECODE_OK)
    {
        boat_radio_parser_reset(parser);
        return BOAT_RADIO_PARSE_FRAME;
    }

    boat_radio_parser_seek_next_sync(parser);
    return BOAT_RADIO_PARSE_ERROR;
}
