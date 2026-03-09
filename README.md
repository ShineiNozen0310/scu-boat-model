# scu-boat-model

A cleaner STM32 RC boat firmware core derived from a common IR remote demo project.

## Why this repo exists

The original Keil project from the provided `D:` source tree works, but it couples too much logic together:

- TIM2 interrupt both decodes IR pulses and mutates boat state
- control state lives in global variables shared across modules
- motor, servo, LED, and OLED logic are mixed into the main loop
- tunables such as speed steps and servo limits are hard-coded in several places

This repository generates a better baseline with:

- interrupt code that only captures raw IR edge intervals
- a standalone NEC decoder that runs outside the ISR
- an explicit controller state for throttle, gear, rudder, and repeat handling
- smooth throttle ramping and safer gear changes
- one config header for limits and timing tolerances
- a small HAL boundary for motor, servo, LED, and display integration

## Repository layout

- `firmware/include/boat_config.h`: tuning constants and IR timing windows
- `firmware/include/boat_remote.h`: remote key map from the original source
- `firmware/include/boat_ir_decoder.h`: NEC decoder API
- `firmware/include/ir_capture_queue.h`: ISR-to-main ring buffer for pulse intervals
- `firmware/include/boat_hal.h`: platform hooks
- `firmware/include/boat_controller.h`: high-level boat state and behavior
- `firmware/src/boat_ir_decoder.c`: NEC interval decoder
- `firmware/src/ir_capture_queue.c`: lightweight queue for capture events
- `firmware/src/boat_controller.c`: controller state machine
- `firmware/src/main.c`: integration example for a Keil/STM32 project
- `docs/porting.md`: how to adapt the code to the original board and project

## Behavior changes from the original demo

- `BTN_0` stops the boat and centers the rudder
- `BTN_*` selects forward gear and resets throttle to zero
- `BTN_#` selects reverse gear and resets throttle to zero
- `BTN_UP` and `BTN_DOWN` adjust throttle in fixed steps
- `BTN_LEFT` and `BTN_RIGHT` adjust rudder in fixed steps
- `BTN_OK` recenters the rudder
- repeated NEC frames are supported for held directional keys
- output throttle ramps toward the target instead of jumping immediately

## Integration notes

The code in `firmware/src/main.c` is intentionally a platform template, not a full STM32 build:

1. Keep your existing STM32 clock, GPIO, timer, OLED, and motor drivers.
2. Replace the `platform_*` stubs with your board-specific code.
3. Feed each TIM2 capture interval into `BoatIrCaptureQueue_Push()`.
4. Call `BoatController_Tick()` in the main loop.
5. Map signed motor output to your H-bridge direction pins and PWM duty cycle.

The original pin mapping from the Bilibili source is documented in `docs/porting.md`.
