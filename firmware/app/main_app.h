#ifndef MAIN_APP_H
#define MAIN_APP_H

#include <stdbool.h>
#include <stdint.h>

#include "boat_command.h"
#include "control/boat_controller.h"
#include "drivers/boat_crsf.h"
#include "drivers/boat_ir_decoder.h"
#include "drivers/boat_radio_protocol.h"
#include "drivers/ir_capture_queue.h"
#include "safety/boat_safety.h"

typedef struct MainAppPlatform
{
    void (*board_init)(void);
    uint32_t (*millis)(void);
    bool (*battery_is_low)(void);
    const BoatHal *hal;
} MainAppPlatform;

typedef struct MainApp
{
    BoatController controller;
    BoatCrsfParser crsf_parser;
    BoatRadioParser radio_parser;
    BoatIrDecoder ir_decoder;
    BoatIrCaptureQueue capture_queue;
    BoatSafety safety;
    uint8_t last_radio_sequence;
    bool has_radio_sequence;
    const MainAppPlatform *platform;
} MainApp;

void MainApp_Init(MainApp *app, const MainAppPlatform *platform);
void MainApp_RunOnce(MainApp *app);
bool MainApp_ApplyCommand(MainApp *app, const BoatCommand *command);
bool MainApp_OnCrsfByte(MainApp *app, uint8_t byte);
bool MainApp_OnCrsfBytes(MainApp *app, const uint8_t *payload, uint16_t length);
bool MainApp_OnCrsfFrame(MainApp *app, const uint8_t *payload, uint8_t length);
bool MainApp_OnRadioByte(MainApp *app, uint8_t byte);
bool MainApp_OnRadioBytes(MainApp *app, const uint8_t *payload, uint16_t length);
bool MainApp_OnRadioPacket(MainApp *app, const uint8_t *payload, uint8_t length);
void MainApp_OnCaptureIntervalUs(MainApp *app, uint16_t interval_us);
void MainApp_SetEmergencyStop(MainApp *app, bool enabled);
const BoatController *MainApp_Controller(const MainApp *app);
const BoatSafety *MainApp_Safety(const MainApp *app);

#endif
