#ifndef BOAT_CONTROLLER_H
#define BOAT_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

#include "../drivers/boat_hal.h"

typedef enum BoatGear
{
    BOAT_GEAR_FORWARD = 1,
    BOAT_GEAR_REVERSE = -1
} BoatGear;

typedef struct BoatController
{
    BoatGear gear;
    uint8_t throttle_target_percent;
    int16_t throttle_output_percent;
    uint8_t rudder_angle_deg;
    uint8_t last_repeatable_command;
    uint32_t last_command_ms;
    bool has_last_repeatable_command;
} BoatController;

void BoatController_Init(BoatController *controller);
bool BoatController_HandleCommand(BoatController *controller, uint8_t ir_command, uint32_t now_ms);
bool BoatController_HandleRepeat(BoatController *controller, uint32_t now_ms);
void BoatController_Tick(BoatController *controller, const BoatHal *hal);
void BoatController_ForceStop(BoatController *controller, bool center_rudder);
int16_t BoatController_TargetSignedPercent(const BoatController *controller);
bool BoatController_IsMoving(const BoatController *controller);

#endif
