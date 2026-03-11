#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"

#include "boat_app_port.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_TIM3_Init();
    MX_USART1_UART_Init();

    BoatApp_Port_Init();

    while (1)
    {
        BoatApp_Port_RunOnce();
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    BoatApp_Port_HalUartRxCpltCallback(huart);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    BoatApp_Port_HalUartErrorCallback(huart);
}
