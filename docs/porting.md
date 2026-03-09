# Porting the generated firmware

## Original hardware mapping

The provided source tree uses these STM32F103 pins:

- IR receiver: `PA0` with `TIM2` input capture
- status LEDs: `PA1` to `PA4`
- servo PWM: `PA6` with `TIM3 CH1`
- motor PWM: `PA8` with `TIM1 CH1`
- motor direction pins: `PB12` and `PB13`

## Recommended integration flow

1. Add all files from `firmware/include` and `firmware/src` to your Keil project.
2. Keep your existing low-level drivers if they already work electrically.
3. Replace the placeholder `platform_*` functions in `firmware/src/main.c`.
4. Keep TIM2 configured in input capture mode and push every measured interval into the queue.
5. Convert the signed controller output into your driver convention:

   - positive value: forward
   - negative value: reverse
   - absolute value: PWM duty in percent

## Mapping old code to new modules

- old `IR_Rcv()` -> `BoatIrDecoder_PushIntervalUs()`
- old `IR_Reply()` -> `BoatController_HandleCommand()` and `BoatController_HandleRepeat()`
- old globals `Motor_Speed`, `Motor_Dir`, `Servo_Angel` -> `BoatController`
- old direct ISR-side state changes -> queue plus main-loop processing

## Why this structure is safer

- The interrupt handler stays short and deterministic.
- Decode failures do not directly corrupt motor or servo state.
- Speed and rudder limits are centralized in one header.
- Repeat frames can be handled without duplicating button logic.
- Future upgrades such as ultrasonic avoidance or PID speed control now have a clear extension point.
