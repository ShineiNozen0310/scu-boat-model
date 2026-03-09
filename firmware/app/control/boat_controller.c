#include "boat_controller.h"

#include "../boat_config.h"
#include "../boat_remote.h"

static uint8_t clamp_percent(int32_t value)
{
    if (value < 0)
    {
        return 0U;
    }

    if (value > (int32_t)BOAT_THROTTLE_MAX_PERCENT)
    {
        return BOAT_THROTTLE_MAX_PERCENT;
    }

    return (uint8_t)value;
}

static uint8_t clamp_angle(int32_t value)
{
    if (value < (int32_t)BOAT_SERVO_MIN_DEG)
    {
        return BOAT_SERVO_MIN_DEG;
    }

    if (value > (int32_t)BOAT_SERVO_MAX_DEG)
    {
        return BOAT_SERVO_MAX_DEG;
    }

    return (uint8_t)value;
}

static int16_t approach_target(int16_t current, int16_t target, int16_t step)
{
    if (current < target)
    {
        current += step;
        if (current > target)
        {
            current = target;
        }
    }
    else if (current > target)
    {
        current -= step;
        if (current < target)
        {
            current = target;
        }
    }

    return current;
}

static bool is_repeatable_command(uint8_t ir_command)
{
    switch (ir_command)
    {
        case BOAT_REMOTE_BTN_UP:
        case BOAT_REMOTE_BTN_DOWN:
        case BOAT_REMOTE_BTN_LEFT:
        case BOAT_REMOTE_BTN_RIGHT:
            return true;

        default:
            return false;
    }
}

static bool apply_motion_command(BoatController *controller, uint8_t ir_command)
{
    uint8_t previous_throttle = controller->throttle_target_percent;
    uint8_t previous_angle = controller->rudder_angle_deg;

    switch (ir_command)
    {
        case BOAT_REMOTE_BTN_UP:
            controller->throttle_target_percent = clamp_percent(
                (int32_t)controller->throttle_target_percent + (int32_t)BOAT_THROTTLE_STEP_PERCENT);
            break;

        case BOAT_REMOTE_BTN_DOWN:
            controller->throttle_target_percent = clamp_percent(
                (int32_t)controller->throttle_target_percent - (int32_t)BOAT_THROTTLE_STEP_PERCENT);
            break;

        case BOAT_REMOTE_BTN_LEFT:
            controller->rudder_angle_deg = clamp_angle(
                (int32_t)controller->rudder_angle_deg + (int32_t)BOAT_SERVO_STEP_DEG);
            break;

        case BOAT_REMOTE_BTN_RIGHT:
            controller->rudder_angle_deg = clamp_angle(
                (int32_t)controller->rudder_angle_deg - (int32_t)BOAT_SERVO_STEP_DEG);
            break;

        default:
            return false;
    }

    return previous_throttle != controller->throttle_target_percent ||
           previous_angle != controller->rudder_angle_deg;
}

int16_t BoatController_TargetSignedPercent(const BoatController *controller)
{
    int16_t target = (int16_t)controller->throttle_target_percent;

    if (controller->gear == BOAT_GEAR_REVERSE)
    {
        target = (int16_t)(-target);
    }

    return target;
}

bool BoatController_IsMoving(const BoatController *controller)
{
    return controller->throttle_output_percent != 0;
}

void BoatController_Init(BoatController *controller)
{
    controller->gear = BOAT_GEAR_FORWARD;
    controller->throttle_target_percent = 0U;
    controller->throttle_output_percent = 0;
    controller->rudder_angle_deg = BOAT_SERVO_CENTER_DEG;
    controller->last_repeatable_command = 0U;
    controller->last_command_ms = 0U;
    controller->has_last_repeatable_command = false;
}

void BoatController_ForceStop(BoatController *controller, bool center_rudder)
{
    controller->throttle_target_percent = 0U;
    controller->throttle_output_percent = 0;
    controller->has_last_repeatable_command = false;

    if (center_rudder)
    {
        controller->rudder_angle_deg = BOAT_SERVO_CENTER_DEG;
    }
}

bool BoatController_HandleCommand(BoatController *controller, uint8_t ir_command, uint32_t now_ms)
{
    bool changed = false;

    switch (ir_command)
    {
        case BOAT_REMOTE_BTN_0:
            changed = controller->gear != BOAT_GEAR_FORWARD ||
                      controller->throttle_target_percent != 0U ||
                      controller->rudder_angle_deg != BOAT_SERVO_CENTER_DEG ||
                      controller->throttle_output_percent != 0;
            controller->gear = BOAT_GEAR_FORWARD;
            BoatController_ForceStop(controller, true);
            break;

        case BOAT_REMOTE_BTN_STAR:
            changed = controller->gear != BOAT_GEAR_FORWARD ||
                      controller->throttle_target_percent != 0U;
            controller->gear = BOAT_GEAR_FORWARD;
            controller->throttle_target_percent = 0U;
            controller->has_last_repeatable_command = false;
            break;

        case BOAT_REMOTE_BTN_WELL:
            changed = controller->gear != BOAT_GEAR_REVERSE ||
                      controller->throttle_target_percent != 0U;
            controller->gear = BOAT_GEAR_REVERSE;
            controller->throttle_target_percent = 0U;
            controller->has_last_repeatable_command = false;
            break;

        case BOAT_REMOTE_BTN_OK:
            changed = controller->rudder_angle_deg != BOAT_SERVO_CENTER_DEG;
            controller->rudder_angle_deg = BOAT_SERVO_CENTER_DEG;
            controller->has_last_repeatable_command = false;
            break;

        default:
            changed = apply_motion_command(controller, ir_command);
            if (is_repeatable_command(ir_command))
            {
                controller->last_repeatable_command = ir_command;
                controller->has_last_repeatable_command = true;
            }
            else if (!changed)
            {
                return false;
            }
            break;
    }

    controller->last_command_ms = now_ms;
    return changed;
}

bool BoatController_HandleRepeat(BoatController *controller, uint32_t now_ms)
{
    if (!controller->has_last_repeatable_command)
    {
        return false;
    }

    if ((uint32_t)(now_ms - controller->last_command_ms) > BOAT_REPEAT_WINDOW_MS)
    {
        controller->has_last_repeatable_command = false;
        return false;
    }

    controller->last_command_ms = now_ms;
    return apply_motion_command(controller, controller->last_repeatable_command);
}

void BoatController_Tick(BoatController *controller, const BoatHal *hal)
{
    int16_t target_output;
    bool front_led;
    bool rear_led;
    bool left_led;
    bool right_led;

    target_output = BoatController_TargetSignedPercent(controller);
    controller->throttle_output_percent = approach_target(
        controller->throttle_output_percent,
        target_output,
        (int16_t)BOAT_THROTTLE_RAMP_STEP_PERCENT);

    front_led = controller->throttle_output_percent > 0;
    rear_led = controller->throttle_output_percent < 0;
    left_led = controller->rudder_angle_deg > BOAT_SERVO_CENTER_DEG;
    right_led = controller->rudder_angle_deg < BOAT_SERVO_CENTER_DEG;

    if (hal != 0 && hal->motor_set_signed_percent != 0)
    {
        hal->motor_set_signed_percent(controller->throttle_output_percent);
    }

    if (hal != 0 && hal->servo_set_angle_deg != 0)
    {
        hal->servo_set_angle_deg(controller->rudder_angle_deg);
    }

    if (hal != 0 && hal->led_set != 0)
    {
        hal->led_set(front_led, rear_led, left_led, right_led);
    }

    if (hal != 0 && hal->display_state != 0)
    {
        hal->display_state(controller);
    }
}
