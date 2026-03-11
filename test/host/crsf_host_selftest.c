#include <stdbool.h>
#include <stdint.h>

#include "../../firmware/app/boat_config.h"
#include "../../firmware/app/main_app.h"

enum
{
    TEST_CRSF_PACKET_LENGTH = 26U,
    TEST_CRSF_FRAME_LENGTH = 24U
};

typedef struct TestHalState
{
    int16_t motor_percent;
    uint8_t rudder_angle_deg;
    uint8_t turret_yaw_angle_deg;
    uint8_t turret_pitch_angle_deg;
    bool water_cannon_enabled;
    bool led_front;
    bool led_rear;
    bool led_left;
    bool led_right;
} TestHalState;

static TestHalState g_hal_state;
static uint32_t g_now_ms;
static bool g_battery_low;

static void *test_memset(void *destination, uint8_t value, uint32_t length)
{
    uint8_t *bytes = (uint8_t *)destination;
    uint32_t index;

    for (index = 0U; index < length; ++index)
    {
        bytes[index] = value;
    }

    return destination;
}

static void *test_memcpy(void *destination, const void *source, uint32_t length)
{
    uint8_t *dst = (uint8_t *)destination;
    const uint8_t *src = (const uint8_t *)source;
    uint32_t index;

    for (index = 0U; index < length; ++index)
    {
        dst[index] = src[index];
    }

    return destination;
}

static void reset_hal_state(void)
{
    (void)test_memset(&g_hal_state, 0U, (uint32_t)sizeof(g_hal_state));
}

static void test_motor_set_signed_percent(int16_t signed_percent)
{
    g_hal_state.motor_percent = signed_percent;
}

static void test_rudder_set_angle_deg(uint8_t angle_deg)
{
    g_hal_state.rudder_angle_deg = angle_deg;
}

static void test_turret_yaw_set_angle_deg(uint8_t angle_deg)
{
    g_hal_state.turret_yaw_angle_deg = angle_deg;
}

static void test_turret_pitch_set_angle_deg(uint8_t angle_deg)
{
    g_hal_state.turret_pitch_angle_deg = angle_deg;
}

static void test_water_cannon_set_enabled(bool enabled)
{
    g_hal_state.water_cannon_enabled = enabled;
}

static void test_led_set(bool front, bool rear, bool left, bool right)
{
    g_hal_state.led_front = front;
    g_hal_state.led_rear = rear;
    g_hal_state.led_left = left;
    g_hal_state.led_right = right;
}

static uint32_t test_millis(void)
{
    return g_now_ms;
}

static bool test_battery_is_low(void)
{
    return g_battery_low;
}

static const BoatHal g_test_hal = {
    test_motor_set_signed_percent,
    test_rudder_set_angle_deg,
    test_turret_yaw_set_angle_deg,
    test_turret_pitch_set_angle_deg,
    test_water_cannon_set_enabled,
    test_led_set,
    0
};

static const MainAppPlatform g_test_platform = {
    0,
    test_millis,
    test_battery_is_low,
    &g_test_hal
};

#define CHECK(condition)                                                                       \
    do                                                                                         \
    {                                                                                          \
        if (!(condition))                                                                      \
        {                                                                                      \
            return false;                                                                      \
        }                                                                                      \
    } while (0)

static uint16_t test_us_to_ticks(int16_t pulse_us)
{
    return (uint16_t)((((int32_t)pulse_us - 1500) * 8) / 5 + 992);
}

static void test_pack_channels(const uint16_t *channels, uint8_t *payload)
{
    uint8_t channel_index;
    uint8_t payload_index = 0U;
    uint32_t accumulator = 0U;
    uint8_t bits_in_accumulator = 0U;

    (void)test_memset(payload, 0U, BOAT_CRSF_RC_PAYLOAD_LENGTH);

    for (channel_index = 0U; channel_index < BOAT_CRSF_NUM_CHANNELS; ++channel_index)
    {
        accumulator |= ((uint32_t)(channels[channel_index] & 0x07FFU) << bits_in_accumulator);
        bits_in_accumulator = (uint8_t)(bits_in_accumulator + 11U);

        while (bits_in_accumulator >= 8U)
        {
            payload[payload_index++] = (uint8_t)(accumulator & 0xFFU);
            accumulator >>= 8U;
            bits_in_accumulator = (uint8_t)(bits_in_accumulator - 8U);
        }
    }

    if (payload_index < BOAT_CRSF_RC_PAYLOAD_LENGTH)
    {
        payload[payload_index++] = (uint8_t)(accumulator & 0xFFU);
    }
}

static void build_rc_channels_packet(const uint16_t *channels, uint8_t *packet)
{
    packet[0] = BOAT_CRSF_SYNC_BYTE;
    packet[1] = TEST_CRSF_FRAME_LENGTH;
    packet[2] = BOAT_CRSF_FRAME_RC_CHANNELS_PACKED;
    test_pack_channels(channels, &packet[3]);
    packet[TEST_CRSF_PACKET_LENGTH - 1U] = BoatCrsf_Crc8(&packet[2], TEST_CRSF_FRAME_LENGTH - 1U);
}

static bool test_decode_frame_unpacks_all_16_channels(void)
{
    uint8_t packet[TEST_CRSF_PACKET_LENGTH];
    uint16_t expected[BOAT_CRSF_NUM_CHANNELS] = {
        192U, 432U, 672U, 912U,
        1152U, 1392U, 1632U, 1792U,
        210U, 450U, 690U, 930U,
        1170U, 1410U, 1650U, 1792U
    };
    BoatCrsfFrame frame;
    uint8_t index;

    build_rc_channels_packet(expected, packet);
    CHECK(BoatCrsfParser_DecodeFrame(packet, sizeof(packet), &frame) == BOAT_CRSF_PARSE_RC_CHANNELS);

    for (index = 0U; index < BOAT_CRSF_NUM_CHANNELS; ++index)
    {
        CHECK(frame.channels.value[index] == expected[index]);
    }

    return true;
}

static bool test_streaming_bytes_updates_controller_state(void)
{
    MainApp app;
    uint8_t packet[TEST_CRSF_PACKET_LENGTH];
    uint8_t stream[TEST_CRSF_PACKET_LENGTH + 3U];
    uint16_t channels[BOAT_CRSF_NUM_CHANNELS];
    const BoatController *controller;
    uint8_t index;

    for (index = 0U; index < BOAT_CRSF_NUM_CHANNELS; ++index)
    {
        channels[index] = test_us_to_ticks(1500);
    }

    channels[BOAT_CRSF_CHANNEL_RUDDER - 1U] = test_us_to_ticks(2000);
    channels[BOAT_CRSF_CHANNEL_THROTTLE - 1U] = test_us_to_ticks(1000);
    channels[BOAT_CRSF_CHANNEL_TURRET_YAW - 1U] = test_us_to_ticks(1500);
    channels[BOAT_CRSF_CHANNEL_TURRET_PITCH - 1U] = test_us_to_ticks(2000);
    channels[BOAT_CRSF_CHANNEL_WATER_CANNON - 1U] = test_us_to_ticks(2000);
    channels[BOAT_CRSF_CHANNEL_LIGHTS - 1U] = test_us_to_ticks(2000);
    channels[BOAT_CRSF_CHANNEL_EMERGENCY_STOP - 1U] = test_us_to_ticks(1000);

    reset_hal_state();
    g_now_ms = 0U;
    g_battery_low = false;
    MainApp_Init(&app, &g_test_platform);

    build_rc_channels_packet(channels, packet);
    stream[0] = 0x00U;
    stream[1] = 0x55U;
    stream[2] = 0xAAU;
    (void)test_memcpy(&stream[3], packet, (uint32_t)sizeof(packet));

    CHECK(MainApp_OnCrsfBytes(&app, stream, sizeof(stream)));

    controller = MainApp_Controller(&app);
    CHECK(controller != 0);
    CHECK(BoatController_TargetSignedPercent(controller) == -100);
    CHECK(controller->rudder_angle_deg == BOAT_SERVO_MAX_DEG);
    CHECK(controller->turret_yaw_angle_deg == BOAT_TURRET_YAW_CENTER_DEG);
    CHECK(controller->turret_pitch_angle_deg == BOAT_TURRET_PITCH_MAX_DEG);
    CHECK(controller->water_cannon_enabled);
    CHECK(controller->navigation_lights_enabled);

    MainApp_RunOnce(&app);
    CHECK(g_hal_state.motor_percent == -(int16_t)BOAT_THROTTLE_RAMP_STEP_PERCENT);
    CHECK(g_hal_state.rudder_angle_deg == BOAT_SERVO_MAX_DEG);
    CHECK(g_hal_state.turret_pitch_angle_deg == BOAT_TURRET_PITCH_MAX_DEG);
    CHECK(g_hal_state.water_cannon_enabled);
    CHECK(g_hal_state.led_front);
    CHECK(g_hal_state.led_rear);
    CHECK(g_hal_state.led_left);
    CHECK(g_hal_state.led_right);

    return true;
}

static bool test_emergency_stop_and_link_loss(void)
{
    MainApp app;
    uint8_t packet[TEST_CRSF_PACKET_LENGTH];
    uint16_t channels[BOAT_CRSF_NUM_CHANNELS];
    const BoatController *controller;
    const BoatSafety *safety;
    uint8_t index;

    for (index = 0U; index < BOAT_CRSF_NUM_CHANNELS; ++index)
    {
        channels[index] = test_us_to_ticks(1500);
    }

    reset_hal_state();
    g_now_ms = 0U;
    g_battery_low = false;
    MainApp_Init(&app, &g_test_platform);

    channels[BOAT_CRSF_CHANNEL_THROTTLE - 1U] = test_us_to_ticks(2000);
    channels[BOAT_CRSF_CHANNEL_EMERGENCY_STOP - 1U] = test_us_to_ticks(2000);
    build_rc_channels_packet(channels, packet);
    CHECK(MainApp_OnCrsfFrame(&app, packet, sizeof(packet)));

    controller = MainApp_Controller(&app);
    CHECK(controller != 0);
    CHECK(BoatController_TargetSignedPercent(controller) == 0);

    MainApp_RunOnce(&app);
    safety = MainApp_Safety(&app);
    CHECK(safety != 0);
    CHECK(BoatSafety_ActiveReason(safety) == BOAT_SAFETY_REASON_EMERGENCY_STOP);

    channels[BOAT_CRSF_CHANNEL_THROTTLE - 1U] = test_us_to_ticks(2000);
    channels[BOAT_CRSF_CHANNEL_EMERGENCY_STOP - 1U] = test_us_to_ticks(1000);
    g_now_ms = 10U;
    build_rc_channels_packet(channels, packet);
    CHECK(MainApp_OnCrsfFrame(&app, packet, sizeof(packet)));
    MainApp_RunOnce(&app);
    CHECK(BoatSafety_ActiveReason(MainApp_Safety(&app)) == BOAT_SAFETY_REASON_NONE);
    CHECK(BoatController_TargetSignedPercent(MainApp_Controller(&app)) == 100);

    g_now_ms = BOAT_COMMAND_TIMEOUT_MS + 20U;
    MainApp_RunOnce(&app);
    CHECK(BoatSafety_ActiveReason(MainApp_Safety(&app)) == BOAT_SAFETY_REASON_LINK_LOSS);
    CHECK(BoatController_TargetSignedPercent(MainApp_Controller(&app)) == 0);
    CHECK(g_hal_state.motor_percent == 0);

    return true;
}

int main(void)
{
    if (!test_decode_frame_unpacks_all_16_channels())
    {
        return 1;
    }

    if (!test_streaming_bytes_updates_controller_state())
    {
        return 1;
    }

    if (!test_emergency_stop_and_link_loss())
    {
        return 1;
    }
    return 0;
}
