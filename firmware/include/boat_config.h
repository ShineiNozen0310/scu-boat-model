#ifndef BOAT_CONFIG_H
#define BOAT_CONFIG_H

#define BOAT_THROTTLE_MAX_PERCENT        100U
#define BOAT_THROTTLE_STEP_PERCENT       20U
#define BOAT_THROTTLE_RAMP_STEP_PERCENT  4U

#define BOAT_SERVO_CENTER_DEG            90U
#define BOAT_SERVO_MIN_DEG               30U
#define BOAT_SERVO_MAX_DEG               150U
#define BOAT_SERVO_STEP_DEG              15U

#define BOAT_REPEAT_WINDOW_MS            220U
#define BOAT_COMMAND_TIMEOUT_MS          0U

#define BOAT_IR_START_US                 13500U
#define BOAT_IR_REPEAT_US                11250U
#define BOAT_IR_BIT0_US                  1120U
#define BOAT_IR_BIT1_US                  2250U
#define BOAT_IR_START_TOLERANCE_US       2500U
#define BOAT_IR_REPEAT_TOLERANCE_US      1500U
#define BOAT_IR_BIT_TOLERANCE_US         350U

#define BOAT_IR_QUEUE_CAPACITY           48U

#endif
