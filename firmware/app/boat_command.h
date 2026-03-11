#ifndef BOAT_COMMAND_H
#define BOAT_COMMAND_H

#include <stdbool.h>
#include <stdint.h>

enum
{
    BOAT_COMMAND_FLAG_WATER_CANNON_ENABLED = 1U << 0,
    BOAT_COMMAND_FLAG_LIGHTS_ENABLED = 1U << 1
};

typedef struct BoatCommand
{
    int8_t throttle_percent;
    int8_t rudder_percent;
    int8_t turret_yaw_percent;
    int8_t turret_pitch_percent;
    uint8_t flags;
} BoatCommand;

static inline void BoatCommand_InitNeutral(BoatCommand *command)
{
    if (command != 0)
    {
        command->throttle_percent = 0;
        command->rudder_percent = 0;
        command->turret_yaw_percent = 0;
        command->turret_pitch_percent = 0;
        command->flags = 0U;
    }
}

static inline bool BoatCommand_WaterCannonEnabled(const BoatCommand *command)
{
    return command != 0 &&
           (command->flags & BOAT_COMMAND_FLAG_WATER_CANNON_ENABLED) != 0U;
}

static inline bool BoatCommand_LightsEnabled(const BoatCommand *command)
{
    return command != 0 &&
           (command->flags & BOAT_COMMAND_FLAG_LIGHTS_ENABLED) != 0U;
}

#endif
