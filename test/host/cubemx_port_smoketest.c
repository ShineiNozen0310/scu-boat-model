#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../../firmware/cubemx/main.h"
#include "../../firmware/cubemx/gpio.h"
#include "../../firmware/cubemx/tim.h"
#include "../../firmware/cubemx/usart.h"
#include "../../firmware/cubemx/boat_app_port.h"
#include "../../firmware/cubemx/boat_board_config.h"
#include "../../firmware/app/drivers/boat_radio_protocol.h"

TIM_HandleTypeDef htim3 = { {71U} };
UART_HandleTypeDef huart1 = { 0U };

static uint32_t g_tick_ms;
static uint8_t *g_rx_buffer;
static uint16_t g_rx_size;
static uint32_t g_uart_receive_it_calls;
static GPIO_PinState g_aux_pin_state = GPIO_PIN_SET;
static const char *g_failed_condition;
static uint32_t g_failed_line;

#define CHECK(condition)                                                                       \
    do                                                                                         \
    {                                                                                          \
        if (!(condition))                                                                      \
        {                                                                                      \
            g_failed_condition = #condition;                                                   \
            g_failed_line = (uint32_t)__LINE__;                                                \
            return false;                                                                      \
        }                                                                                      \
    } while (0)

static int report_failed_test(const char *test_name)
{
    (void)fprintf(stderr, "FAILED: %s (line %lu): %s\n", test_name, (unsigned long)g_failed_line, g_failed_condition);
    return 1;
}

void HAL_Init(void)
{
}

void SystemClock_Config(void)
{
}

void MX_GPIO_Init(void)
{
}

void MX_TIM3_Init(void)
{
}

void MX_USART1_UART_Init(void)
{
}

HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size)
{
    CHECK(huart == &huart1);
    CHECK(data != 0);
    CHECK(size == 1U);
    g_rx_buffer = data;
    g_rx_size = size;
    ++g_uart_receive_it_calls;
    return 0;
}

HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t channel)
{
    (void)htim;
    (void)channel;
    return 0;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *gpio_port, uint16_t gpio_pin, GPIO_PinState pin_state)
{
    (void)gpio_port;
    (void)gpio_pin;
    (void)pin_state;
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *gpio_port, uint16_t gpio_pin)
{
    (void)gpio_port;
    (void)gpio_pin;
    return g_aux_pin_state;
}

uint32_t HAL_GetTick(void)
{
    return g_tick_ms++;
}

uint32_t HAL_RCC_GetPCLK1Freq(void)
{
    return 36000000U;
}

void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *clock_config, uint32_t *flash_latency)
{
    if (clock_config != 0)
    {
        clock_config->APB1CLKDivider = RCC_HCLK_DIV1;
    }

    if (flash_latency != 0)
    {
        *flash_latency = 0U;
    }
}

static bool build_radio_packet(
    uint8_t sequence,
    int8_t throttle_percent,
    int8_t rudder_percent,
    int8_t turret_yaw_percent,
    int8_t turret_pitch_percent,
    uint8_t flags,
    uint8_t *packet)
{
    BoatRadioFrame frame;

    BoatRadioFrame_InitNeutral(&frame);
    frame.sequence = sequence;
    frame.command.throttle_percent = throttle_percent;
    frame.command.rudder_percent = rudder_percent;
    frame.command.turret_yaw_percent = turret_yaw_percent;
    frame.command.turret_pitch_percent = turret_pitch_percent;
    frame.command.flags = flags;

    CHECK(packet != 0);
    CHECK(BoatRadioProtocol_Encode(&frame, packet, BOAT_RADIO_FRAME_LENGTH));
    return true;
}

static bool test_boat_app_port_consumes_streamed_radio_bytes(void)
{
    uint8_t packet[BOAT_RADIO_FRAME_LENGTH];
    const BoatController *controller;
    uint8_t index;
    uint32_t calls_before_error;

    g_tick_ms = 0U;
    g_rx_buffer = 0;
    g_rx_size = 0U;
    g_uart_receive_it_calls = 0U;
    g_aux_pin_state = GPIO_PIN_SET;

    BoatApp_Port_Init();
    CHECK(g_uart_receive_it_calls == 1U);
    CHECK(g_rx_buffer != 0);
    CHECK(g_rx_size == 1U);

    CHECK(build_radio_packet(7U, 55, -25, 35, -10, BOAT_COMMAND_FLAG_LIGHTS_ENABLED, packet));

    for (index = 0U; index < BOAT_RADIO_FRAME_LENGTH; ++index)
    {
        *g_rx_buffer = packet[index];
        BoatApp_Port_HalUartRxCpltCallback(&huart1);
    }

    controller = MainApp_Controller(&g_boat_app);
    CHECK(controller != 0);
    CHECK(BoatController_TargetSignedPercent(controller) == 55);
    CHECK(controller->rudder_angle_deg < BOAT_SERVO_CENTER_DEG);
    CHECK(controller->turret_yaw_angle_deg > BOAT_TURRET_YAW_CENTER_DEG);
    CHECK(controller->turret_pitch_angle_deg < BOAT_TURRET_PITCH_CENTER_DEG);
    CHECK(controller->navigation_lights_enabled);
    CHECK(g_uart_receive_it_calls == (uint32_t)(BOAT_RADIO_FRAME_LENGTH + 1U));

    calls_before_error = g_uart_receive_it_calls;
    BoatApp_Port_HalUartErrorCallback(&huart1);
    CHECK(g_uart_receive_it_calls == calls_before_error + 1U);

    return true;
}

int main(void)
{
    if (!test_boat_app_port_consumes_streamed_radio_bytes())
    {
        return report_failed_test("test_boat_app_port_consumes_streamed_radio_bytes");
    }

    return 0;
}
