#ifndef MAIN_APP_H
#define MAIN_APP_H

#include <stdbool.h>
#include <stdint.h>

#include "control/boat_controller.h"
#include "drivers/boat_ir_decoder.h"
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
    BoatIrDecoder ir_decoder;
    BoatIrCaptureQueue capture_queue;
    BoatSafety safety;
    const MainAppPlatform *platform;
} MainApp;

void MainApp_Init(MainApp *app, const MainAppPlatform *platform);
void MainApp_RunOnce(MainApp *app);
void MainApp_OnCaptureIntervalUs(MainApp *app, uint16_t interval_us);
void MainApp_SetEmergencyStop(MainApp *app, bool enabled);
const BoatController *MainApp_Controller(const MainApp *app);
const BoatSafety *MainApp_Safety(const MainApp *app);

#endif
