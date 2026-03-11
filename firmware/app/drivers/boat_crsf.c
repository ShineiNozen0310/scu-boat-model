#include "boat_crsf.h"

static void boat_crsf_reset(BoatCrsfParser *parser)
{
    parser->index = 0U;
    parser->expected_length = 0U;
}

static void boat_crsf_decode_channels(const uint8_t *payload, BoatCrsfChannels *channels)
{
    uint8_t channel_index;
    uint8_t byte_index = 0U;
    uint32_t accumulator = 0U;
    uint8_t bits_in_accumulator = 0U;

    for (channel_index = 0U; channel_index < BOAT_CRSF_NUM_CHANNELS; ++channel_index)
    {
        while (bits_in_accumulator < 11U && byte_index < BOAT_CRSF_RC_PAYLOAD_LENGTH)
        {
            accumulator |= ((uint32_t)payload[byte_index] << bits_in_accumulator);
            bits_in_accumulator = (uint8_t)(bits_in_accumulator + 8U);
            ++byte_index;
        }

        channels->value[channel_index] = (uint16_t)(accumulator & 0x07FFU);
        accumulator >>= 11U;
        bits_in_accumulator = (uint8_t)(bits_in_accumulator - 11U);
    }
}

void BoatCrsfParser_Init(BoatCrsfParser *parser)
{
    if (parser != 0)
    {
        boat_crsf_reset(parser);
    }
}

uint8_t BoatCrsf_Crc8(const uint8_t *data, uint8_t length)
{
    uint8_t crc = 0U;
    uint8_t index;

    if (data == 0)
    {
        return 0U;
    }

    for (index = 0U; index < length; ++index)
    {
        uint8_t bit_index;

        crc ^= data[index];
        for (bit_index = 0U; bit_index < 8U; ++bit_index)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1U) ^ 0xD5U);
            }
            else
            {
                crc <<= 1U;
            }
        }
    }

    return crc;
}

int16_t BoatCrsf_TicksToUs(uint16_t ticks)
{
    return (int16_t)((((int32_t)ticks - 992) * 5) / 8 + 1500);
}

BoatCrsfParseResult BoatCrsfParser_DecodeFrame(
    const uint8_t *packet,
    uint8_t length,
    BoatCrsfFrame *out_frame)
{
    uint8_t frame_length;
    uint8_t payload_length;
    uint8_t crc_index;

    if (packet == 0 || out_frame == 0 || length < 4U)
    {
        return BOAT_CRSF_PARSE_ERROR;
    }

    if (packet[0] != BOAT_CRSF_SYNC_BYTE)
    {
        return BOAT_CRSF_PARSE_ERROR;
    }

    frame_length = packet[1];
    if (frame_length < 2U || length != (uint8_t)(frame_length + 2U))
    {
        return BOAT_CRSF_PARSE_ERROR;
    }

    crc_index = (uint8_t)(length - 1U);
    if (packet[crc_index] != BoatCrsf_Crc8(&packet[2], (uint8_t)(frame_length - 1U)))
    {
        return BOAT_CRSF_PARSE_ERROR;
    }

    out_frame->address = packet[0];
    out_frame->length = frame_length;
    out_frame->type = packet[2];

    payload_length = (uint8_t)(frame_length - 2U);
    if (out_frame->type != BOAT_CRSF_FRAME_RC_CHANNELS_PACKED)
    {
        return BOAT_CRSF_PARSE_IGNORED_FRAME;
    }

    if (payload_length != BOAT_CRSF_RC_PAYLOAD_LENGTH)
    {
        return BOAT_CRSF_PARSE_ERROR;
    }

    boat_crsf_decode_channels(&packet[3], &out_frame->channels);
    return BOAT_CRSF_PARSE_RC_CHANNELS;
}

BoatCrsfParseResult BoatCrsfParser_PushByte(BoatCrsfParser *parser, uint8_t byte, BoatCrsfFrame *out_frame)
{
    if (parser == 0)
    {
        return BOAT_CRSF_PARSE_ERROR;
    }

    if (parser->index == 0U)
    {
        if (byte != BOAT_CRSF_SYNC_BYTE)
        {
            return BOAT_CRSF_PARSE_NONE;
        }

        parser->buffer[parser->index++] = byte;
        return BOAT_CRSF_PARSE_NONE;
    }

    if (parser->index >= BOAT_CRSF_MAX_PACKET_LENGTH)
    {
        boat_crsf_reset(parser);
        return BOAT_CRSF_PARSE_ERROR;
    }

    parser->buffer[parser->index++] = byte;

    if (parser->index == 2U)
    {
        uint8_t frame_length = parser->buffer[1];

        if (frame_length < 2U ||
            frame_length > (uint8_t)(BOAT_CRSF_MAX_PACKET_LENGTH - 2U))
        {
            boat_crsf_reset(parser);
            return BOAT_CRSF_PARSE_ERROR;
        }

        parser->expected_length = (uint8_t)(frame_length + 2U);
        return BOAT_CRSF_PARSE_NONE;
    }

    if (parser->expected_length != 0U &&
        parser->index == parser->expected_length)
    {
        BoatCrsfParseResult result = BoatCrsfParser_DecodeFrame(
            parser->buffer,
            parser->expected_length,
            out_frame);
        boat_crsf_reset(parser);
        return result;
    }

    return BOAT_CRSF_PARSE_NONE;
}
