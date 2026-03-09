#ifndef BOAT_SAFETY_H
#define BOAT_SAFETY_H

#include <stdbool.h>
#include <stdint.h>

#include "../control/boat_controller.h"

typedef enum BoatSafetyReason
{
    BOAT_SAFETY_REASON_NONE = 0,
    BOAT_SAFETY_REASON_LINK_LOSS,
    BOAT_SAFETY_REASON_LOW_VOLTAGE,
    BOAT_SAFETY_REASON_EMERGENCY_STOP
} BoatSafetyReason;

typedef struct BoatSafety
{
    uint32_t last_rx_ms;
    BoatSafetyReason active_reason;
    bool has_rx;
    bool emergency_stop;
} BoatSafety;

void BoatSafety_Init(BoatSafety *safety);
void BoatSafety_NotifyRx(BoatSafety *safety, uint32_t now_ms);
void BoatSafety_SetEmergencyStop(BoatSafety *safety, bool enabled);
void BoatSafety_Tick(BoatSafety *safety, BoatController *controller, uint32_t now_ms, bool battery_low);
BoatSafetyReason BoatSafety_ActiveReason(const BoatSafety *safety);
bool BoatSafety_IsDriveBlocked(const BoatSafety *safety);
const char *BoatSafety_ReasonString(BoatSafetyReason reason);

#endif
