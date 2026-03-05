#include "services/PressData.h"

static volatile bool     PressReady     = false;
static volatile uint32_t LastDurationMs = 0;

static PressStats Stats = {0, 0, 0, 0};

void SetPressResult(uint32_t durationMs) {
    LastDurationMs = durationMs;
    PressReady     = true;
}

bool HasPressResult() {
    return PressReady;
}

bool ConsumePressResult(uint32_t* outMs) {
    if (!PressReady) return false;
    *outMs     = LastDurationMs;
    PressReady = false;
    return true;
}

void RecordPress(uint32_t durationMs) {
    Stats.TotalPresses++;
    Stats.TotalDurationMs += durationMs;
    if (durationMs < ShortPressThresholdMs) {
        Stats.ShortPresses++;
    } else {
        Stats.LongPresses++;
    }
}

PressStats GetStats() {
    return Stats;
}

void ResetStats() {
    Stats.TotalPresses    = 0;
    Stats.ShortPresses    = 0;
    Stats.LongPresses     = 0;
    Stats.TotalDurationMs = 0;
}
