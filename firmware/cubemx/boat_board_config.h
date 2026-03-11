#ifndef BOAT_BOARD_CONFIG_H
#define BOAT_BOARD_CONFIG_H

/*
 * Recommended STM32F103 pin plan for the current project:
 *
 * USART1
 *   PA10 RX <- CRSF receiver TX
 *   PA9  TX -> CRSF receiver RX
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

/*
 * ExpressLRS receivers commonly use 420000 baud over CRSF.
 * Switch this to 416666 if you later move to a strict TBS Crossfire setup.
 */
#define BOAT_CRSF_UART_BAUD                  420000U

#define BOAT_ENABLE_LED_OUTPUTS              1
#define BOAT_ENABLE_BATTERY_ADC              0
#define BOAT_BATTERY_LOW_THRESHOLD_MV        7000U
#define BOAT_BATTERY_DIVIDER_SCALE_MILLIVOLT 2U

#endif
