#ifndef BOAT_APP_PORT_H
#define BOAT_APP_PORT_H

#include <stdbool.h>
#include <stdint.h>

#include "usart.h"
#include "../app/main_app.h"

extern MainApp g_boat_app;

void BoatApp_Port_Init(void);
void BoatApp_Port_RunOnce(void);
void BoatApp_Port_SetEmergencyStop(bool enabled);
void BoatApp_Port_OnCrsfRxByte(uint8_t byte);
void BoatApp_Port_OnCrsfRxBuffer(const uint8_t *buffer, uint16_t length);
void BoatApp_Port_OnRadioRxByte(uint8_t byte);
void BoatApp_Port_OnRadioRxBuffer(const uint8_t *buffer, uint16_t length);
void BoatApp_Port_HalUartRxCpltCallback(UART_HandleTypeDef *huart);
void BoatApp_Port_HalUartErrorCallback(UART_HandleTypeDef *huart);

#endif
