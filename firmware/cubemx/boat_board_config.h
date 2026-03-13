#ifndef BOAT_BOARD_CONFIG_H
#define BOAT_BOARD_CONFIG_H

/*
 * Recommended STM32F103 pin plan for the current project:
 *
 * USART1
 *   PA10 RX <- CRSF receiver TX / E220 TXD
 *   PA9  TX -> CRSF receiver RX / E220 RXD
 *   PB12   -> optional E220 M0 (GPIO output, default low)
 *   PB13   -> optional E220 M1 (GPIO output, default low)
 *   PB14   <- optional E220 AUX (GPIO input, high when ready)
 *
 * TIM3 PWM @ 50Hz
 *   PA6 CH1 -> bidirectional brushed ESC throttle
 *   PA7 CH2 -> rudder servo
 *   PB0 CH3 -> turret yaw servo
 *   PB1 CH4 -> turret pitch servo
 *
 * GPIO output
 *   PA8      -> water cannon MOSFET gate
 *   PA1..PA4 -> optional LEDs
 *
 * Optional ADC
 *   PA5 -> battery divider input
 */

#define BOAT_ESC_PULSE_MIN_US                1000U
#define BOAT_ESC_PULSE_CENTER_US             1500U
#define BOAT_ESC_PULSE_MAX_US                2000U

#define BOAT_SERVO_PULSE_MIN_US              1000U
#define BOAT_SERVO_PULSE_CENTER_US           1500U
#define BOAT_SERVO_PULSE_MAX_US              2000U

#define BOAT_UART_LINK_MODE_CRSF             1U
#define BOAT_UART_LINK_MODE_RADIO_PACKET     2U

/*
 * Default to the budget radio-packet link used by the current under-800 BOM.
 * Switch this back to BOAT_UART_LINK_MODE_CRSF if you later move to ELRS/CRSF.
 */
#define BOAT_UART_LINK_MODE                  BOAT_UART_LINK_MODE_RADIO_PACKET

/*
 * ExpressLRS receivers commonly use 420000 baud over CRSF.
 * Switch this to 416666 if you later move to a strict TBS Crossfire setup.
 */
#define BOAT_CRSF_UART_BAUD                  420000U

/*
 * E220 transparent UART modules default to a much lower serial baud rate.
 * Keep this in sync with the module UART configuration on both ends.
 */
#define BOAT_RADIO_UART_BAUD                 9600U

/*
 * Optional E220 mode/ready pins. Leave disabled if you hard-wire the module in
 * transparent normal mode, which is sufficient for the current budget link.
 */
#define BOAT_ENABLE_RADIO_MODE_PINS          0
#define BOAT_RADIO_M0_GPIO_PORT              GPIOB
#define BOAT_RADIO_M0_PIN                    GPIO_PIN_12
#define BOAT_RADIO_M1_GPIO_PORT              GPIOB
#define BOAT_RADIO_M1_PIN                    GPIO_PIN_13

#define BOAT_ENABLE_RADIO_AUX_INPUT          0
#define BOAT_RADIO_AUX_GPIO_PORT             GPIOB
#define BOAT_RADIO_AUX_PIN                   GPIO_PIN_14
#define BOAT_RADIO_AUX_READY_STATE           GPIO_PIN_SET
#define BOAT_RADIO_BOOT_WAIT_MS              50U
#define BOAT_RADIO_AUX_WAIT_TIMEOUT_MS       20U

#if BOAT_UART_LINK_MODE == BOAT_UART_LINK_MODE_CRSF
#define BOAT_UART1_BAUD                      BOAT_CRSF_UART_BAUD
#elif BOAT_UART_LINK_MODE == BOAT_UART_LINK_MODE_RADIO_PACKET
#define BOAT_UART1_BAUD                      BOAT_RADIO_UART_BAUD
#else
#error Unsupported BOAT_UART_LINK_MODE
#endif

#define BOAT_ENABLE_LED_OUTPUTS              1
#define BOAT_ENABLE_BATTERY_ADC              0
#define BOAT_BATTERY_LOW_THRESHOLD_MV        7000U
#define BOAT_BATTERY_DIVIDER_SCALE_MILLIVOLT 2U

#endif
