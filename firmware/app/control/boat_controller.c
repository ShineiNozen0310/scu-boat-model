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

static uint8_t clamp_angle(int32_t value, uint8_t min_angle_deg, uint8_t max_angle_deg)
{
    if (value < (int32_t)min_angle_deg)
    {
        return min_angle_deg;
    }

    if (value > (int32_t)max_angle_deg)
    {
        return max_angle_deg;
    }

    return (uint8_t)value;
}

static int8_t clamp_axis_percent(int32_t value)
{
    if (value < -100)
    {
        return -100;
    }

    if (value > 100)
    {
        return 100;
    }

    return (int8_t)value;
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

static void set_signed_throttle_target(BoatController *controller, int16_t signed_percent)
{
    if (signed_percent < 0)
    {
        controller->gear = BOAT_GEAR_REVERSE;
        controller->throttle_target_percent = clamp_percent(-signed_percent);
        return;
    }

    controller->gear = BOAT_GEAR_FORWARD;
    controller->throttle_target_percent = clamp_percent(signed_percent);
}

static uint8_t map_axis_percent_to_angle(
    int8_t axis_percent,
    uint8_t min_angle_deg,
    uint8_t center_angle_deg,
    uint8_t max_angle_deg)
{
    int32_t span;
    int32_t angle;
    int32_t clamped_percent = (int32_t)clamp_axis_percent(axis_percent);

    if (clamped_percent >= 0)
    {
        span = (int32_t)max_angle_deg - (int32_t)center_angle_deg;
    }
    else
    {
        span = (int32_t)center_angle_deg - (int32_t)min_angle_deg;
    }

    angle = (int32_t)center_angle_deg + (clamped_percent * span) / 100;
    return clamp_angle(angle, min_angle_deg, max_angle_deg);
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
                (int32_t)controller->rudder_angle_deg + (int32_t)BOAT_SERVO_STEP_DEG,
                BOAT_SERVO_MIN_DEG,
                BOAT_SERVO_MAX_DEG);
            break;

        case BOAT_REMOTE_BTN_RIGHT:
            controller->rudder_angle_deg = clamp_angle(
                (int32_t)controller->rudder_angle_deg - (int32_t)BOAT_SERVO_STEP_DEG,
                BOAT_SERVO_MIN_DEG,
                BOAT_SERVO_MAX_DEG);
            break;

        default:
            return false;
    }

    return previous_throttle != controller->throttle_target_percent ||
           previous_angle != controller->rudder_angle_deg;
}

static bool controller_state_changed(
    int16_t previous_signed_target,
    uint8_t previous_rudder,
    uint8_t previous_turret_yaw,
    uint8_t previous_turret_pitch,
    bool previous_water_cannon,
    bool previous_navigation_lights,
    const BoatController *controller)
{
    return previous_signed_target != BoatController_TargetSignedPercent(controller) ||
           previous_rudder != controller->rudder_angle_deg ||
           previous_turret_yaw != controller->turret_yaw_angle_deg ||
           previous_turret_pitch != controller->turret_pitch_angle_deg ||
           previous_water_cannon != controller->water_cannon_enabled ||
           previous_navigation_lights != controller->navigation_lights_enabled;
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
    controller->turret_yaw_angle_deg = BOAT_TURRET_YAW_CENTER_DEG;
    controller->turret_pitch_angle_deg = BOAT_TURRET_PITCH_CENTER_DEG;
    controller->water_cannon_enabled = false;
    controller->navigation_lights_enabled = false;
    controller->last_repeatable_command = 0U;
    controller->last_command_ms = 0U;
    controller->has_last_repeatable_command = false;
}

bool BoatController_ApplyCommand(BoatController *controller, const BoatCommand *command, uint32_t now_ms)
{
    int16_t previous_signed_target;
    uint8_t previous_rudder;
    uint8_t previous_turret_yaw;
    uint8_t previous_turret_pitch;
    bool previous_water_cannon;
    bool previous_navigation_lights;

    if (controller == 0 || command == 0)
    {
        return false;
    }

    previous_signed_target = BoatController_TargetSignedPercent(controller);
    previous_rudder = controller->rudder_angle_deg;
    previous_turret_yaw = controller->turret_yaw_angle_deg;
    previous_turret_pitch = controller->turret_pitch_angle_deg;
    previous_water_cannon = controller->water_cannon_enabled;
    previous_navigation_lights = controller->navigation_lights_enabled;

    set_signed_throttle_target(controller, (int16_t)clamp_axis_percent(command->throttle_percent));
    controller->rudder_angle_deg = map_axis_percent_to_angle(
        command->rudder_percent,
        BOAT_SERVO_MIN_DEG,
        BOAT_SERVO_CENTER_DEG,
        BOAT_SERVO_MAX_DEG);
    controller->turret_yaw_angle_deg = map_axis_percent_to_angle(
        command->turret_yaw_percent,
        BOAT_TURRET_YAW_MIN_DEG,
        BOAT_TURRET_YAW_CENTER_DEG,
        BOAT_TURRET_YAW_MAX_DEG);
    controller->turret_pitch_angle_deg = map_axis_percent_to_angle(
        command->turret_pitch_percent,
        BOAT_TURRET_PITCH_MIN_DEG,
        BOAT_TURRET_PITCH_CENTER_DEG,
        BOAT_TURRET_PITCH_MAX_DEG);
    controller->water_cannon_enabled = BoatCommand_WaterCannonEnabled(command);
    controller->navigation_lights_enabled = BoatCommand_LightsEnabled(command);
    controller->has_last_repeatable_command = false;
    controller->last_command_ms = now_ms;

    return controller_state_changed(
        previous_signed_target,
        previous_rudder,
        previous_turret_yaw,
        previous_turret_pitch,
        previous_water_cannon,
        previous_navigation_lights,
        controller);
}

void BoatController_ForceStop(BoatController *controller, bool center_rudder)
{
    controller->throttle_target_percent = 0U;
    controller->throttle_output_percent = 0;
    controller->water_cannon_enabled = false;
    controller->has_last_repeatable_command = false;

    if (center_rudder)
    {
        controller->rudder_angle_deg = BOAT_SERVO_CENTER_DEG;
        controller->turret_yaw_angle_deg = BOAT_TURRET_YAW_CENTER_DEG;
        controller->turret_pitch_angle_deg = BOAT_TURRET_PITCH_CENTER_DEG;
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
                      controller->turret_yaw_angle_deg != BOAT_TURRET_YAW_CENTER_DEG ||
                      controller->turret_pitch_angle_deg != BOAT_TURRET_PITCH_CENTER_DEG ||
                      controller->water_cannon_enabled ||
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

    if (controller->navigation_lights_enabled)
    {
        front_led = true;
        rear_led = true;
        left_led = true;
        right_led = true;
    }

    if (hal != 0 && hal->motor_set_signed_percent != 0)
    {
        hal->motor_set_signed_percent(controller->throttle_output_percent);
    }

    if (hal != 0 && hal->servo_set_angle_deg != 0)
    {
        hal->servo_set_angle_deg(controller->rudder_angle_deg);
    }

    if (hal != 0 && hal->turret_yaw_set_angle_deg != 0)
    {
        hal->turret_yaw_set_angle_deg(controller->turret_yaw_angle_deg);
    }

    if (hal != 0 && hal->turret_pitch_set_angle_deg != 0)
    {
        hal->turret_pitch_set_angle_deg(controller->turret_pitch_angle_deg);
    }

    if (hal != 0 && hal->water_cannon_set_enabled != 0)
    {
        hal->water_cannon_set_enabled(controller->water_cannon_enabled);
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
