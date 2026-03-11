#ifndef ADC_H
#define ADC_H

#include "main.h"

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
