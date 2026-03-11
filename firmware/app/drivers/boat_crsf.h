#ifndef BOAT_CRSF_H
#define BOAT_CRSF_H

#include <stdint.h>

#define BOAT_CRSF_SYNC_BYTE                0xC8U
#define BOAT_CRSF_MAX_PACKET_LENGTH        64U
#define BOAT_CRSF_NUM_CHANNELS             16U
#define BOAT_CRSF_FRAME_RC_CHANNELS_PACKED 0x16U
#define BOAT_CRSF_RC_PAYLOAD_LENGTH        22U

typedef enum BoatCrsfParseResult
{
    BOAT_CRSF_PARSE_NONE = 0,
    BOAT_CRSF_PARSE_RC_CHANNELS,
    BOAT_CRSF_PARSE_IGNORED_FRAME,
    BOAT_CRSF_PARSE_ERROR
} BoatCrsfParseResult;

typedef struct BoatCrsfChannels
{
    uint16_t value[BOAT_CRSF_NUM_CHANNELS];
} BoatCrsfChannels;

typedef struct BoatCrsfFrame
{
    uint8_t address;
    uint8_t length;
    uint8_t type;
    BoatCrsfChannels channels;
} BoatCrsfFrame;

typedef struct BoatCrsfParser
{
    uint8_t buffer[BOAT_CRSF_MAX_PACKET_LENGTH];
    uint8_t index;
    uint8_t expected_length;
} BoatCrsfParser;

void BoatCrsfParser_Init(BoatCrsfParser *parser);
BoatCrsfParseResult BoatCrsfParser_PushByte(BoatCrsfParser *parser, uint8_t byte, BoatCrsfFrame *out_frame);
BoatCrsfParseResult BoatCrsfParser_DecodeFrame(
    const uint8_t *packet,
    uint8_t length,
    BoatCrsfFrame *out_frame);
uint8_t BoatCrsf_Crc8(const uint8_t *data, uint8_t length);
int16_t BoatCrsf_TicksToUs(uint16_t ticks);

#endif
