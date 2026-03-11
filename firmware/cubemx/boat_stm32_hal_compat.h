#ifndef BOAT_STM32_HAL_COMPAT_H
#define BOAT_STM32_HAL_COMPAT_H

#include <stdint.h>

#if defined(__has_include)
#define BOAT_HAL_HAS_INCLUDE(path) __has_include(path)
#else
#define BOAT_HAL_HAS_INCLUDE(path) 0
#endif

#if BOAT_HAL_HAS_INCLUDE("main.h")
#include "main.h"
#else
typedef int HAL_StatusTypeDef;

void HAL_Init(void);
void SystemClock_Config(void);
#endif

#if BOAT_HAL_HAS_INCLUDE("gpio.h")
#include "gpio.h"
#else
typedef struct GPIO_TypeDef GPIO_TypeDef;

typedef enum GPIO_PinState
{
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
} GPIO_PinState;

#ifndef GPIOA
#define GPIOA ((GPIO_TypeDef *)0)
#endif

#ifndef GPIO_PIN_1
#define GPIO_PIN_1 ((uint16_t)0x0002U)
#endif

#ifndef GPIO_PIN_2
#define GPIO_PIN_2 ((uint16_t)0x0004U)
#endif

#ifndef GPIO_PIN_3
#define GPIO_PIN_3 ((uint16_t)0x0008U)
#endif

#ifndef GPIO_PIN_4
#define GPIO_PIN_4 ((uint16_t)0x0010U)
#endif

#ifndef GPIO_PIN_5
#define GPIO_PIN_5 ((uint16_t)0x0020U)
#endif

#ifndef GPIO_PIN_8
#define GPIO_PIN_8 ((uint16_t)0x0100U)
#endif

void MX_GPIO_Init(void);
void HAL_GPIO_WritePin(GPIO_TypeDef *gpio_port, uint16_t gpio_pin, GPIO_PinState pin_state);
#endif

#if BOAT_HAL_HAS_INCLUDE("tim.h")
#include "tim.h"
#else
typedef struct TIM_Base_InitTypeDef
{
    uint32_t Prescaler;
} TIM_Base_InitTypeDef;

typedef struct __TIM_HandleTypeDef
{
    TIM_Base_InitTypeDef Init;
} TIM_HandleTypeDef;

extern TIM_HandleTypeDef htim3;

#ifndef TIM_CHANNEL_1
#define TIM_CHANNEL_1 0x00000000U
#endif

#ifndef TIM_CHANNEL_2
#define TIM_CHANNEL_2 0x00000004U
#endif

#ifndef TIM_CHANNEL_3
#define TIM_CHANNEL_3 0x00000008U
#endif

#ifndef TIM_CHANNEL_4
#define TIM_CHANNEL_4 0x0000000CU
#endif

#ifndef __HAL_TIM_SET_COMPARE
#define __HAL_TIM_SET_COMPARE(htim, channel, compare) ((void)(htim), (void)(channel), (void)(compare))
#endif

void MX_TIM3_Init(void);
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t channel);
#endif

#if BOAT_HAL_HAS_INCLUDE("usart.h")
#include "usart.h"
#else
#ifndef BOAT_UART_HANDLE_TYPEDEF_DEFINED
#define BOAT_UART_HANDLE_TYPEDEF_DEFINED
typedef struct __UART_HandleTypeDef
{
    uint32_t reserved;
} UART_HandleTypeDef;
#endif

extern UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void);
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size);
#endif

#if BOAT_HAL_HAS_INCLUDE("adc.h")
#include "adc.h"
#else
typedef struct __ADC_HandleTypeDef
{
    uint32_t reserved;
} ADC_HandleTypeDef;

extern ADC_HandleTypeDef hadc1;

HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *hadc, uint32_t timeout);
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc);
#endif

#if !BOAT_HAL_HAS_INCLUDE("main.h") && !BOAT_HAL_HAS_INCLUDE("tim.h")
#ifndef RCC_HCLK_DIV1
#define RCC_HCLK_DIV1 0U
#endif

typedef struct RCC_ClkInitTypeDef
{
    uint32_t APB1CLKDivider;
} RCC_ClkInitTypeDef;

uint32_t HAL_GetTick(void);
uint32_t HAL_RCC_GetPCLK1Freq(void);
void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *clock_config, uint32_t *flash_latency);
#endif

#undef BOAT_HAL_HAS_INCLUDE

#endif
