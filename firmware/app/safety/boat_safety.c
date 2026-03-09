#include "boat_safety.h"

#include "../boat_config.h"

static BoatSafetyReason evaluate_reason(const BoatSafety *safety, uint32_t now_ms, bool battery_low)
{
    if (safety->emergency_stop)
    {
        return BOAT_SAFETY_REASON_EMERGENCY_STOP;
    }

    if (battery_low)
    {
        return BOAT_SAFETY_REASON_LOW_VOLTAGE;
    }

#if BOAT_COMMAND_TIMEOUT_MS > 0U
    if (safety->has_rx &&
        (uint32_t)(now_ms - safety->last_rx_ms) > BOAT_COMMAND_TIMEOUT_MS)
    {
        return BOAT_SAFETY_REASON_LINK_LOSS;
    }
#else
    (void)now_ms;
#endif

    return BOAT_SAFETY_REASON_NONE;
}

void BoatSafety_Init(BoatSafety *safety)
{
    safety->last_rx_ms = 0U;
    safety->active_reason = BOAT_SAFETY_REASON_NONE;
    safety->has_rx = false;
    safety->emergency_stop = false;
}

void BoatSafety_NotifyRx(BoatSafety *safety, uint32_t now_ms)
{
    safety->last_rx_ms = now_ms;
    safety->has_rx = true;
}

void BoatSafety_SetEmergencyStop(BoatSafety *safety, bool enabled)
{
    safety->emergency_stop = enabled;
}

void BoatSafety_Tick(BoatSafety *safety, BoatController *controller, uint32_t now_ms, bool battery_low)
{
    safety->active_reason = evaluate_reason(safety, now_ms, battery_low);

    if (safety->active_reason != BOAT_SAFETY_REASON_NONE)
    {
        bool center_rudder = safety->active_reason == BOAT_SAFETY_REASON_EMERGENCY_STOP;
        BoatController_ForceStop(controller, center_rudder);
    }
}

BoatSafetyReason BoatSafety_ActiveReason(const BoatSafety *safety)
{
    return safety->active_reason;
}

bool BoatSafety_IsDriveBlocked(const BoatSafety *safety)
{
    return safety->active_reason != BOAT_SAFETY_REASON_NONE;
}

const char *BoatSafety_ReasonString(BoatSafetyReason reason)
{
    switch (reason)
    {
        case BOAT_SAFETY_REASON_LINK_LOSS:
            return "link_loss";

        case BOAT_SAFETY_REASON_LOW_VOLTAGE:
            return "low_voltage";

        case BOAT_SAFETY_REASON_EMERGENCY_STOP:
            return "emergency_stop";

        case BOAT_SAFETY_REASON_NONE:
        default:
            return "ok";
    }
}
