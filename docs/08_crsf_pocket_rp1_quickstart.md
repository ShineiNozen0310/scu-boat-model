# Pocket + RP1 V2 Quickstart

Updated: 2026-03-14

## Scope

This repo now defaults to the `ELRS transmitter -> RP1 V2 -> CRSF -> STM32F103`
path.

The default board switch is:

- `BOAT_UART_LINK_MODE = BOAT_UART_LINK_MODE_CRSF`

See [boat_board_config.h](/c:/Users/LEGION/Desktop/scu-boat-model/firmware/cubemx/boat_board_config.h#L43).

## Wiring

Use a stable `5V UBEC` for the receiver and servos.

| RP1 V2 | STM32F103 / boat side | Note |
| --- | --- | --- |
| `5V` | `UBEC 5V` | Do not power RP1 from raw 2S battery voltage |
| `GND` | `MCU GND` | Common ground is required |
| `TX` | `PA10 / USART1_RX` | Receiver output into MCU RX |
| `RX` | `PA9 / USART1_TX` | Optional for fuller CRSF compatibility, recommended to wire |

## PWM Outputs

The current board mapping stays the same:

| Function | Pin | Output type |
| --- | --- | --- |
| ESC throttle | `PA6 / TIM3_CH1` | `50Hz` PWM |
| Rudder servo | `PA7 / TIM3_CH2` | `50Hz` PWM |
| Turret yaw servo | `PB0 / TIM3_CH3` | `50Hz` PWM |
| Turret pitch servo | `PB1 / TIM3_CH4` | `50Hz` PWM |
| Water cannon MOSFET | `PA8` | GPIO high/low |

## CRSF Channel Map

Recommended `Pocket / EdgeTX Mode 2` map:

| Channel | Function |
| --- | --- |
| `CH1` | Rudder |
| `CH2` | Turret pitch |
| `CH3` | Throttle |
| `CH4` | Turret yaw |
| `CH5` | Water cannon enable |
| `CH6` | Lights |
| `CH7` | Emergency stop |

See [boat_config.h](/c:/Users/LEGION/Desktop/scu-boat-model/firmware/app/boat_config.h#L28).

Recommended transmitter source layout:

| Output channel | Pocket control |
| --- | --- |
| `CH1` | Right stick horizontal (`Ail`) |
| `CH2` | Right stick vertical (`Ele`) |
| `CH3` | Left stick vertical (`Thr`) |
| `CH4` | Left stick horizontal (`Rud`) |
| `CH5` | Water cannon switch |
| `CH6` | Lights switch |
| `CH7` | Emergency stop switch |

## Important Electrical Note

The firmware can switch the water cannon motor through a MOSFET, but it does
not regulate pump voltage.

If your water cannon motor is the `S05 3.7V` unit from the Taobao list, do not
connect it directly to the raw `2S 7.4V` main battery without a suitable power
solution.
