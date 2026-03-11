#ifndef BOAT_HAL_H
#define BOAT_HAL_H

#include <stdbool.h>
#include <stdint.h>

struct BoatController;

typedef struct BoatHal
{
    void (*motor_set_signed_percent)(int16_t signed_percent);
    void (*servo_set_angle_deg)(uint8_t angle_deg);
    void (*turret_yaw_set_angle_deg)(uint8_t angle_deg);
    void (*turret_pitch_set_angle_deg)(uint8_t angle_deg);
    void (*water_cannon_set_enabled)(bool enabled);
    void (*led_set)(bool front, bool rear, bool left, bool right);
    void (*display_state)(const struct BoatController *controller);
} BoatHal;

#endif
