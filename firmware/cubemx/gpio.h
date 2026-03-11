#ifndef GPIO_H
#define GPIO_H

#include "main.h"

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
