#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

typedef int HAL_StatusTypeDef;

#ifndef RCC_HCLK_DIV1
#define RCC_HCLK_DIV1 0U
#endif

typedef struct RCC_ClkInitTypeDef
{
    uint32_t APB1CLKDivider;
} RCC_ClkInitTypeDef;

void HAL_Init(void);
void SystemClock_Config(void);
uint32_t HAL_GetTick(void);
uint32_t HAL_RCC_GetPCLK1Freq(void);
void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *clock_config, uint32_t *flash_latency);

#endif
