#include "main_app.h"

static uint32_t main_app_now_ms(const MainApp *app)
{
    if (app->platform != 0 && app->platform->millis != 0)
    {
        return app->platform->millis();
    }

    return 0U;
}

static bool main_app_battery_is_low(const MainApp *app)
{
    if (app->platform != 0 && app->platform->battery_is_low != 0)
    {
        return app->platform->battery_is_low();
    }

    return false;
}

static const BoatHal *main_app_hal(const MainApp *app)
{
    if (app->platform != 0)
    {
        return app->platform->hal;
    }

    return 0;
}

static void process_ir_input(MainApp *app, uint32_t now_ms)
{
    uint16_t interval_us;

    while (BoatIrCaptureQueue_Pop(&app->capture_queue, &interval_us))
    {
        uint32_t frame = 0U;
        BoatIrDecodeResult result = BoatIrDecoder_PushIntervalUs(&app->ir_decoder, interval_us, &frame);

        if (result == BOAT_IR_RESULT_FRAME && BoatIrDecoder_FrameIsValid(frame))
        {
            BoatSafety_NotifyRx(&app->safety, now_ms);
            (void)BoatController_HandleCommand(&app->controller, BoatIrDecoder_Command(frame), now_ms);
        }
        else if (result == BOAT_IR_RESULT_REPEAT)
        {
            BoatSafety_NotifyRx(&app->safety, now_ms);
            (void)BoatController_HandleRepeat(&app->controller, now_ms);
        }
    }

    if (BoatIrCaptureQueue_ConsumeOverflow(&app->capture_queue))
    {
        BoatIrDecoder_Init(&app->ir_decoder);
    }
}

void MainApp_Init(MainApp *app, const MainAppPlatform *platform)
{
    app->platform = platform;

    BoatController_Init(&app->controller);
    BoatIrDecoder_Init(&app->ir_decoder);
    BoatIrCaptureQueue_Init(&app->capture_queue);
    BoatSafety_Init(&app->safety);

    if (app->platform != 0 && app->platform->board_init != 0)
    {
        app->platform->board_init();
    }
}

void MainApp_RunOnce(MainApp *app)
{
    uint32_t now_ms = main_app_now_ms(app);

    process_ir_input(app, now_ms);
    BoatSafety_Tick(&app->safety, &app->controller, now_ms, main_app_battery_is_low(app));
    BoatController_Tick(&app->controller, main_app_hal(app));
}

void MainApp_OnCaptureIntervalUs(MainApp *app, uint16_t interval_us)
{
    (void)BoatIrCaptureQueue_Push(&app->capture_queue, interval_us);
}

void MainApp_SetEmergencyStop(MainApp *app, bool enabled)
{
    BoatSafety_SetEmergencyStop(&app->safety, enabled);
}

const BoatController *MainApp_Controller(const MainApp *app)
{
    return &app->controller;
}

const BoatSafety *MainApp_Safety(const MainApp *app)
{
    return &app->safety;
}
