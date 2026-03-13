#ifndef BOAT_RADIO_PROTOCOL_H
#define BOAT_RADIO_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#include "../boat_command.h"

#define BOAT_RADIO_FRAME_LENGTH           8U
#define BOAT_RADIO_FRAME_SYNC             0x42U
#define BOAT_RADIO_FRAME_PAYLOAD_LENGTH   (BOAT_RADIO_FRAME_LENGTH - 1U)

typedef enum BoatRadioDecodeStatus
{
    BOAT_RADIO_DECODE_OK = 0,
    BOAT_RADIO_DECODE_INVALID_ARG,
    BOAT_RADIO_DECODE_INVALID_LENGTH,
    BOAT_RADIO_DECODE_INVALID_SYNC,
    BOAT_RADIO_DECODE_INVALID_CHECKSUM
} BoatRadioDecodeStatus;

typedef enum BoatRadioParseResult
{
    BOAT_RADIO_PARSE_NONE = 0,
    BOAT_RADIO_PARSE_FRAME,
    BOAT_RADIO_PARSE_ERROR
} BoatRadioParseResult;

typedef struct BoatRadioFrame
{
    BoatCommand command;
    uint8_t sequence;
} BoatRadioFrame;

typedef struct BoatRadioParser
{
    uint8_t buffer[BOAT_RADIO_FRAME_LENGTH];
    uint8_t index;
} BoatRadioParser;

void BoatRadioFrame_InitNeutral(BoatRadioFrame *frame);
uint8_t BoatRadioProtocol_Checksum(const uint8_t *payload, uint8_t length_without_checksum);
bool BoatRadioProtocol_Encode(const BoatRadioFrame *frame, uint8_t *payload, uint8_t length);
BoatRadioDecodeStatus BoatRadioProtocol_Decode(
    const uint8_t *payload,
    uint8_t length,
    BoatRadioFrame *out_frame);
void BoatRadioParser_Init(BoatRadioParser *parser);
BoatRadioParseResult BoatRadioParser_PushByte(
    BoatRadioParser *parser,
    uint8_t byte,
    BoatRadioFrame *out_frame);

#endif
