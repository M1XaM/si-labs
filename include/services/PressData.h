#pragma once
#include <Arduino.h>

// Duration threshold separating short from long presses.
const uint16_t ShortPressThresholdMs = 500;

struct PressStats {
    uint32_t TotalPresses;
    uint32_t ShortPresses;
    uint32_t LongPresses;
    uint32_t TotalDurationMs;
};

// Called by Task1 when a press completes.
void      SetPressResult(uint32_t durationMs);

// Returns true if an unconsumed press result is waiting (non-destructive).
bool      HasPressResult();

// Called by Task2: returns true and fills *outMs if a new result is ready.
// Clears the ready flag on success.
bool      ConsumePressResult(uint32_t* outMs);

// Called by Task2 to accumulate statistics.
void      RecordPress(uint32_t durationMs);

// Returns a snapshot of the current statistics.
PressStats GetStats();

// Resets all statistics to zero.
void       ResetStats();
