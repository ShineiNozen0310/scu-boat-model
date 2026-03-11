#ifndef USART_H
#define USART_H

#include "main.h"

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
