#ifndef TIM_H
#define TIM_H

#include "main.h"

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
