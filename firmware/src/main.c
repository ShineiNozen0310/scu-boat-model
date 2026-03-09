#include "boat_controller.h"
#include "boat_ir_decoder.h"
#include "ir_capture_queue.h"

static BoatController g_controller;
static BoatIrDecoder g_ir_decoder;
static BoatIrCaptureQueue g_capture_queue;

static void platform_init(void);
static uint32_t platform_millis(void);
static uint16_t platform_tim2_capture_interval_us(void);
static void platform_motor_set_signed_percent(int16_t signed_percent);
static void platform_servo_set_angle_deg(uint8_t angle_deg);
static void platform_led_set(bool front, bool rear, bool left, bool right);
static void platform_display_state(const BoatController *controller);

static const BoatHal g_hal =
{
    platform_motor_set_signed_percent,
    platform_servo_set_angle_deg,
    platform_led_set,
    platform_display_state
};

static void process_ir_input(uint32_t now_ms)
{
    uint16_t interval_us;

    while (BoatIrCaptureQueue_Pop(&g_capture_queue, &interval_us))
    {
        uint32_t frame = 0U;
        BoatIrDecodeResult result = BoatIrDecoder_PushIntervalUs(&g_ir_decoder, interval_us, &frame);

        if (result == BOAT_IR_RESULT_FRAME && BoatIrDecoder_FrameIsValid(frame))
        {
            (void)BoatController_HandleCommand(&g_controller, BoatIrDecoder_Command(frame), now_ms);
        }
        else if (result == BOAT_IR_RESULT_REPEAT)
        {
            (void)BoatController_HandleRepeat(&g_controller, now_ms);
        }
    }

    if (BoatIrCaptureQueue_ConsumeOverflow(&g_capture_queue))
    {
        BoatIrDecoder_Init(&g_ir_decoder);
    }
}

int main(void)
{
    platform_init();

    BoatController_Init(&g_controller);
    BoatIrDecoder_Init(&g_ir_decoder);
    BoatIrCaptureQueue_Init(&g_capture_queue);

    for (;;)
    {
        uint32_t now_ms = platform_millis();
        process_ir_input(now_ms);
        BoatController_Tick(&g_controller, now_ms, &g_hal);
    }
}

void TIM2_IRQHandler(void)
{
    const uint16_t interval_us = platform_tim2_capture_interval_us();
    (void)BoatIrCaptureQueue_Push(&g_capture_queue, interval_us);
}

static void platform_init(void)
{
    /* TODO: initialize system clock, GPIO, PWM, OLED, and TIM2 input capture. */
}

static uint32_t platform_millis(void)
{
    /* TODO: return a monotonic millisecond tick, for example from SysTick. */
    return 0U;
}

static uint16_t platform_tim2_capture_interval_us(void)
{
    /* TODO: read the captured TIM2 interval, reset the timer, and clear the IRQ flag. */
    return 0U;
}

static void platform_motor_set_signed_percent(int16_t signed_percent)
{
    (void)signed_percent;
    /* TODO: original board uses TIM1 CH1 on PA8 plus PB12/PB13 for direction. */
    /* TODO: positive is forward, negative is reverse, absolute value is PWM duty percent. */
}

static void platform_servo_set_angle_deg(uint8_t angle_deg)
{
    (void)angle_deg;
    /* TODO: original demo uses TIM3 CH1 on PA6 with CCR = angle_deg / 5 + 9. */
}

static void platform_led_set(bool front, bool rear, bool left, bool right)
{
    (void)front;
    (void)rear;
    (void)left;
    (void)right;
    /* TODO: drive your PA1-PA4 status LEDs here if needed. */
}

static void platform_display_state(const BoatController *controller)
{
    (void)controller;
    /* TODO: optionally render throttle, gear, rudder angle, and last command on OLED. */
}
