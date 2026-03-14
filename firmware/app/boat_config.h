#ifndef BOAT_CONFIG_H
#define BOAT_CONFIG_H

/* Motion tuning */
#define BOAT_THROTTLE_MAX_PERCENT        100U
#define BOAT_THROTTLE_STEP_PERCENT       20U
#define BOAT_THROTTLE_RAMP_STEP_PERCENT  4U

/* Rudder tuning */
#define BOAT_SERVO_CENTER_DEG            90U
#define BOAT_SERVO_MIN_DEG               30U
#define BOAT_SERVO_MAX_DEG               150U
#define BOAT_SERVO_STEP_DEG              15U

/* Turret tuning */
#define BOAT_TURRET_YAW_CENTER_DEG       90U
#define BOAT_TURRET_YAW_MIN_DEG          15U
#define BOAT_TURRET_YAW_MAX_DEG          165U
#define BOAT_TURRET_PITCH_CENTER_DEG     90U
#define BOAT_TURRET_PITCH_MIN_DEG        45U
#define BOAT_TURRET_PITCH_MAX_DEG        140U

/* Safety tuning */
#define BOAT_REPEAT_WINDOW_MS            220U
#define BOAT_COMMAND_TIMEOUT_MS          1000U

/*
 * Recommended Pocket / EdgeTX Mode 2 mapping:
 *   CH1 (Ail) -> rudder / steering
 *   CH2 (Ele) -> turret pitch
 *   CH3 (Thr) -> throttle
 *   CH4 (Rud) -> turret yaw
 */
#define BOAT_CRSF_CHANNEL_RUDDER         1U
#define BOAT_CRSF_CHANNEL_TURRET_PITCH   2U
#define BOAT_CRSF_CHANNEL_THROTTLE       3U
#define BOAT_CRSF_CHANNEL_TURRET_YAW     4U
#define BOAT_CRSF_CHANNEL_WATER_CANNON   5U
#define BOAT_CRSF_CHANNEL_LIGHTS         6U
#define BOAT_CRSF_CHANNEL_EMERGENCY_STOP 7U

/* CRSF input scaling */
#define BOAT_CRSF_CENTER_US             1500
#define BOAT_CRSF_MIN_US                1000
#define BOAT_CRSF_MAX_US                2000
#define BOAT_CRSF_DEADBAND_US           20
#define BOAT_CRSF_SWITCH_ACTIVE_US      1700

/* NEC timing windows */
#define BOAT_IR_START_US                 13500U
#define BOAT_IR_REPEAT_US                11250U
#define BOAT_IR_BIT0_US                  1120U
#define BOAT_IR_BIT1_US                  2250U
#define BOAT_IR_START_TOLERANCE_US       2500U
#define BOAT_IR_REPEAT_TOLERANCE_US      1500U
#define BOAT_IR_BIT_TOLERANCE_US         350U

#define BOAT_IR_QUEUE_CAPACITY           48U

#endif
