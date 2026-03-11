#include "main_app.h"

#include "boat_config.h"

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

static uint16_t main_app_crsf_channel_ticks(const BoatCrsfChannels *channels, uint8_t channel_number)
{
    if (channels == 0 || channel_number == 0U || channel_number > BOAT_CRSF_NUM_CHANNELS)
    {
        return 0U;
    }

    return channels->value[channel_number - 1U];
}

static int16_t main_app_abs_int16(int16_t value)
{
    if (value < 0)
    {
        return (int16_t)(-value);
    }

    return value;
}

static int8_t main_app_clamp_axis_percent(int32_t value)
{
    if (value < -100)
    {
        return -100;
    }

    if (value > 100)
    {
        return 100;
    }

    return (int8_t)value;
}

static int8_t main_app_crsf_axis_percent(const BoatCrsfChannels *channels, uint8_t channel_number)
{
    int16_t input_us;
    int16_t delta_us;
    int32_t percent;

    input_us = BoatCrsf_TicksToUs(main_app_crsf_channel_ticks(channels, channel_number));
    delta_us = (int16_t)(input_us - BOAT_CRSF_CENTER_US);

    if (main_app_abs_int16(delta_us) <= BOAT_CRSF_DEADBAND_US)
    {
        return 0;
    }

    if (delta_us > 0)
    {
        percent = ((int32_t)delta_us * 100) / (BOAT_CRSF_MAX_US - BOAT_CRSF_CENTER_US);
    }
    else
    {
        percent = ((int32_t)delta_us * 100) / (BOAT_CRSF_CENTER_US - BOAT_CRSF_MIN_US);
    }

    return main_app_clamp_axis_percent(percent);
}

static bool main_app_crsf_switch_active(const BoatCrsfChannels *channels, uint8_t channel_number)
{
    if (channel_number == 0U)
    {
        return false;
    }

    return BoatCrsf_TicksToUs(main_app_crsf_channel_ticks(channels, channel_number)) >=
           BOAT_CRSF_SWITCH_ACTIVE_US;
}

static void main_app_crsf_channels_to_command(
    const BoatCrsfChannels *channels,
    BoatCommand *command,
    bool *emergency_stop)
{
    BoatCommand_InitNeutral(command);

    if (channels == 0 || command == 0)
    {
        if (emergency_stop != 0)
        {
            *emergency_stop = false;
        }
        return;
    }

    command->rudder_percent = main_app_crsf_axis_percent(channels, BOAT_CRSF_CHANNEL_RUDDER);
    command->throttle_percent = main_app_crsf_axis_percent(channels, BOAT_CRSF_CHANNEL_THROTTLE);
    command->turret_yaw_percent = main_app_crsf_axis_percent(channels, BOAT_CRSF_CHANNEL_TURRET_YAW);
    command->turret_pitch_percent = main_app_crsf_axis_percent(channels, BOAT_CRSF_CHANNEL_TURRET_PITCH);

    if (main_app_crsf_switch_active(channels, BOAT_CRSF_CHANNEL_WATER_CANNON))
    {
        command->flags |= BOAT_COMMAND_FLAG_WATER_CANNON_ENABLED;
    }

    if (main_app_crsf_switch_active(channels, BOAT_CRSF_CHANNEL_LIGHTS))
    {
        command->flags |= BOAT_COMMAND_FLAG_LIGHTS_ENABLED;
    }

    if (emergency_stop != 0)
    {
        *emergency_stop = main_app_crsf_switch_active(channels, BOAT_CRSF_CHANNEL_EMERGENCY_STOP);
    }
}

static bool main_app_apply_crsf_channels(MainApp *app, const BoatCrsfChannels *channels)
{
    BoatCommand command;
    bool emergency_stop;

    if (app == 0 || channels == 0)
    {
        return false;
    }

    main_app_crsf_channels_to_command(channels, &command, &emergency_stop);
    MainApp_SetEmergencyStop(app, emergency_stop);

    if (emergency_stop)
    {
        BoatCommand_InitNeutral(&command);
    }

    return MainApp_ApplyCommand(app, &command);
}

bool MainApp_ApplyCommand(MainApp *app, const BoatCommand *command)
{
    uint32_t now_ms;

    if (app == 0 || command == 0)
    {
        return false;
    }

    now_ms = main_app_now_ms(app);
    BoatSafety_NotifyRx(&app->safety, now_ms);
    return BoatController_ApplyCommand(&app->controller, command, now_ms);
}

bool MainApp_OnCrsfByte(MainApp *app, uint8_t byte)
{
    BoatCrsfFrame frame;
    BoatCrsfParseResult result;

    if (app == 0)
    {
        return false;
    }

    result = BoatCrsfParser_PushByte(&app->crsf_parser, byte, &frame);
    if (result != BOAT_CRSF_PARSE_RC_CHANNELS)
    {
        return false;
    }

    return main_app_apply_crsf_channels(app, &frame.channels);
}

bool MainApp_OnCrsfBytes(MainApp *app, const uint8_t *payload, uint16_t length)
{
    uint16_t index;
    bool applied = false;

    if (app == 0 || (payload == 0 && length > 0U))
    {
        return false;
    }

    for (index = 0U; index < length; ++index)
    {
        if (MainApp_OnCrsfByte(app, payload[index]))
        {
            applied = true;
        }
    }

    return applied;
}

bool MainApp_OnCrsfFrame(MainApp *app, const uint8_t *payload, uint8_t length)
{
    BoatCrsfFrame frame;

    if (app == 0)
    {
        return false;
    }

    if (BoatCrsfParser_DecodeFrame(payload, length, &frame) != BOAT_CRSF_PARSE_RC_CHANNELS)
    {
        return false;
    }

    return main_app_apply_crsf_channels(app, &frame.channels);
}

bool MainApp_OnRadioPacket(MainApp *app, const uint8_t *payload, uint8_t length)
{
    BoatRadioFrame frame;

    if (app == 0)
    {
        return false;
    }

    if (BoatRadioProtocol_Decode(payload, length, &frame) != BOAT_RADIO_DECODE_OK)
    {
        return false;
    }

    return MainApp_ApplyCommand(app, &frame.command);
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
    if (app == 0)
    {
        return;
    }

    app->platform = platform;

    BoatController_Init(&app->controller);
    BoatCrsfParser_Init(&app->crsf_parser);
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
    uint32_t now_ms;

    if (app == 0)
    {
        return;
    }

    now_ms = main_app_now_ms(app);

    process_ir_input(app, now_ms);
    BoatSafety_Tick(&app->safety, &app->controller, now_ms, main_app_battery_is_low(app));
    BoatController_Tick(&app->controller, main_app_hal(app));
}

void MainApp_OnCaptureIntervalUs(MainApp *app, uint16_t interval_us)
{
    if (app != 0)
    {
        (void)BoatIrCaptureQueue_Push(&app->capture_queue, interval_us);
    }
}

void MainApp_SetEmergencyStop(MainApp *app, bool enabled)
{
    if (app != 0)
    {
        BoatSafety_SetEmergencyStop(&app->safety, enabled);
    }
}

const BoatController *MainApp_Controller(const MainApp *app)
{
    if (app == 0)
    {
        return 0;
    }

    return &app->controller;
}

const BoatSafety *MainApp_Safety(const MainApp *app)
{
    if (app == 0)
    {
        return 0;
    }

    return &app->safety;
}
