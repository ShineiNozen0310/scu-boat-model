#include "boat_app_port.h"

#include "boat_board_config.h"
#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"

#if BOAT_ENABLE_BATTERY_ADC
#include "adc.h"
#endif

MainApp g_boat_app;

static uint8_t g_uart_rx_byte;

static void boat_uart_start_receive_it(void)
{
    (void)HAL_UART_Receive_IT(&huart1, &g_uart_rx_byte, 1U);
}

static void boat_delay_ms(uint32_t delay_ms)
{
    uint32_t start_ms = HAL_GetTick();

    while ((uint32_t)(HAL_GetTick() - start_ms) < delay_ms)
    {
    }
}

static void boat_radio_prepare_uart_link(void)
{
#if BOAT_UART_LINK_MODE == BOAT_UART_LINK_MODE_RADIO_PACKET
#if BOAT_ENABLE_RADIO_MODE_PINS
    HAL_GPIO_WritePin(BOAT_RADIO_M0_GPIO_PORT, BOAT_RADIO_M0_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BOAT_RADIO_M1_GPIO_PORT, BOAT_RADIO_M1_PIN, GPIO_PIN_RESET);
#endif

#if BOAT_ENABLE_RADIO_AUX_INPUT
    uint32_t start_ms = HAL_GetTick();

    while ((uint32_t)(HAL_GetTick() - start_ms) < BOAT_RADIO_AUX_WAIT_TIMEOUT_MS)
    {
        if (HAL_GPIO_ReadPin(BOAT_RADIO_AUX_GPIO_PORT, BOAT_RADIO_AUX_PIN) ==
            BOAT_RADIO_AUX_READY_STATE)
        {
            return;
        }
    }
#else
    boat_delay_ms(BOAT_RADIO_BOOT_WAIT_MS);
#endif
#endif
}

static uint16_t clamp_u16(uint32_t value, uint16_t min_value, uint16_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }

    if (value > max_value)
    {
        return max_value;
    }

    return (uint16_t)value;
}

static uint16_t map_percent_to_pulse_us(
    int16_t signed_percent,
    uint16_t min_pulse_us,
    uint16_t center_pulse_us,
    uint16_t max_pulse_us)
{
    int32_t pulse_us;

    if (signed_percent >= 0)
    {
        pulse_us = (int32_t)center_pulse_us +
                   ((int32_t)signed_percent * ((int32_t)max_pulse_us - (int32_t)center_pulse_us)) / 100;
    }
    else
    {
        pulse_us = (int32_t)center_pulse_us +
                   ((int32_t)signed_percent * ((int32_t)center_pulse_us - (int32_t)min_pulse_us)) / 100;
    }

    return clamp_u16((uint32_t)pulse_us, min_pulse_us, max_pulse_us);
}

static uint16_t map_angle_to_pulse_us(uint8_t angle_deg)
{
    uint32_t pulse_us = BOAT_SERVO_PULSE_MIN_US +
                        ((uint32_t)angle_deg * (BOAT_SERVO_PULSE_MAX_US - BOAT_SERVO_PULSE_MIN_US)) / 180U;

    return clamp_u16(pulse_us, BOAT_SERVO_PULSE_MIN_US, BOAT_SERVO_PULSE_MAX_US);
}

static uint32_t pwm_compare_from_pulse_us(TIM_HandleTypeDef *htim, uint16_t pulse_us)
{
    RCC_ClkInitTypeDef clk_config;
    uint32_t flash_latency;
    uint32_t timer_clock_hz = HAL_RCC_GetPCLK1Freq();
    uint32_t tick_hz = timer_clock_hz / (htim->Init.Prescaler + 1U);

    HAL_RCC_GetClockConfig(&clk_config, &flash_latency);
    if (clk_config.APB1CLKDivider != RCC_HCLK_DIV1)
    {
        timer_clock_hz *= 2U;
        tick_hz = timer_clock_hz / (htim->Init.Prescaler + 1U);
    }

    return (tick_hz / 1000000U) * pulse_us;
}

static void pwm_set_pulse_us(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t pulse_us)
{
    __HAL_TIM_SET_COMPARE(htim, channel, pwm_compare_from_pulse_us(htim, pulse_us));
}

static void boat_board_init(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    pwm_set_pulse_us(&htim3, TIM_CHANNEL_1, BOAT_ESC_PULSE_CENTER_US);
    pwm_set_pulse_us(&htim3, TIM_CHANNEL_2, BOAT_SERVO_PULSE_CENTER_US);
    pwm_set_pulse_us(&htim3, TIM_CHANNEL_3, BOAT_SERVO_PULSE_CENTER_US);
    pwm_set_pulse_us(&htim3, TIM_CHANNEL_4, BOAT_SERVO_PULSE_CENTER_US);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

#if BOAT_ENABLE_LED_OUTPUTS
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, GPIO_PIN_RESET);
#endif
}

static uint32_t boat_millis(void)
{
    return HAL_GetTick();
}

static bool boat_battery_is_low(void)
{
#if BOAT_ENABLE_BATTERY_ADC
    uint32_t adc_raw;
    uint32_t adc_mv;
    uint32_t battery_mv;

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 5U);
    adc_raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    adc_mv = (adc_raw * 3300U) / 4095U;
    battery_mv = adc_mv * BOAT_BATTERY_DIVIDER_SCALE_MILLIVOLT;
    return battery_mv < BOAT_BATTERY_LOW_THRESHOLD_MV;
#else
    return false;
#endif
}

static void boat_motor_set_signed_percent(int16_t signed_percent)
{
    pwm_set_pulse_us(
        &htim3,
        TIM_CHANNEL_1,
        map_percent_to_pulse_us(
            signed_percent,
            BOAT_ESC_PULSE_MIN_US,
            BOAT_ESC_PULSE_CENTER_US,
            BOAT_ESC_PULSE_MAX_US));
}

static void boat_rudder_set_angle_deg(uint8_t angle_deg)
{
    pwm_set_pulse_us(&htim3, TIM_CHANNEL_2, map_angle_to_pulse_us(angle_deg));
}

static void boat_turret_yaw_set_angle_deg(uint8_t angle_deg)
{
    pwm_set_pulse_us(&htim3, TIM_CHANNEL_3, map_angle_to_pulse_us(angle_deg));
}

static void boat_turret_pitch_set_angle_deg(uint8_t angle_deg)
{
    pwm_set_pulse_us(&htim3, TIM_CHANNEL_4, map_angle_to_pulse_us(angle_deg));
}

static void boat_water_cannon_set_enabled(bool enabled)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void boat_led_set(bool front, bool rear, bool left, bool right)
{
#if BOAT_ENABLE_LED_OUTPUTS
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, front ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, rear ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, left ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, right ? GPIO_PIN_SET : GPIO_PIN_RESET);
#else
    (void)front;
    (void)rear;
    (void)left;
    (void)right;
#endif
}

static const BoatHal g_boat_hal = {
    boat_motor_set_signed_percent,
    boat_rudder_set_angle_deg,
    boat_turret_yaw_set_angle_deg,
    boat_turret_pitch_set_angle_deg,
    boat_water_cannon_set_enabled,
    boat_led_set,
    0
};

static const MainAppPlatform g_boat_platform = {
    boat_board_init,
    boat_millis,
    boat_battery_is_low,
    &g_boat_hal
};

void BoatApp_Port_Init(void)
{
    MainApp_Init(&g_boat_app, &g_boat_platform);
    boat_radio_prepare_uart_link();
    boat_uart_start_receive_it();
}

void BoatApp_Port_RunOnce(void)
{
    MainApp_RunOnce(&g_boat_app);
}

void BoatApp_Port_SetEmergencyStop(bool enabled)
{
    MainApp_SetEmergencyStop(&g_boat_app, enabled);
}

void BoatApp_Port_OnCrsfRxByte(uint8_t byte)
{
    (void)MainApp_OnCrsfByte(&g_boat_app, byte);
}

void BoatApp_Port_OnCrsfRxBuffer(const uint8_t *buffer, uint16_t length)
{
    (void)MainApp_OnCrsfBytes(&g_boat_app, buffer, length);
}

void BoatApp_Port_OnRadioRxByte(uint8_t byte)
{
    (void)MainApp_OnRadioByte(&g_boat_app, byte);
}

void BoatApp_Port_OnRadioRxBuffer(const uint8_t *buffer, uint16_t length)
{
    (void)MainApp_OnRadioBytes(&g_boat_app, buffer, length);
}

void BoatApp_Port_HalUartRxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
#if BOAT_UART_LINK_MODE == BOAT_UART_LINK_MODE_CRSF
        BoatApp_Port_OnCrsfRxByte(g_uart_rx_byte);
#else
        BoatApp_Port_OnRadioRxByte(g_uart_rx_byte);
#endif
        boat_uart_start_receive_it();
    }
}

void BoatApp_Port_HalUartErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        boat_uart_start_receive_it();
    }
}
