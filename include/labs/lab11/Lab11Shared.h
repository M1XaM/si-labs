#pragma once

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

namespace Lab11 {

// ── Hardware pins ───────────────────────────────────────────────────────────
constexpr uint8_t kButtonPin     = 2;
constexpr uint8_t kLedGreenPin   = 12;
constexpr uint8_t kLedRedPin     = 11;

// ── Debounce parameters ─────────────────────────────────────────────────────
// Two matching raw samples confirm a stable logic level. At the 50 ms
// sampling interval this yields a 100 ms validation window — responsive
// short-click detection while still filtering contact bounce.
constexpr uint8_t kDebounceSamples = 2;

// ── Task periods ────────────────────────────────────────────────────────────
constexpr uint16_t kButtonPeriodMs    = 50;
constexpr uint16_t kActuatorPeriodMs  = 50;
constexpr uint16_t kDisplayPeriodMs   = 500;

// ── Finite state machine ────────────────────────────────────────────────────
//
//     +-----------+   press   +-----------+
//     |  Off (G)  | --------> |  On  (R)  |
//     |  (init)   | <-------- |           |
//     +-----------+   press   +-----------+
//
// Pure-logic FSM — no driver calls here. The actuator task translates
// the abstract state into physical LED levels.

enum class FsmStateLab11 : uint8_t {
    Off = 0,
    On  = 1
};

enum class FsmEventLab11 : uint8_t {
    Press = 0
};

FsmStateLab11 stepFsmLab11(FsmStateLab11 current, FsmEventLab11 event);
const char* fsmLabelLab11(FsmStateLab11 state);

// ── Shared runtime state (protected by g_stateMutexLab11) ───────────────────
extern volatile FsmStateLab11 g_ledStateLab11;
extern volatile uint32_t      g_pressCountLab11;

extern SemaphoreHandle_t g_stateMutexLab11;
extern SemaphoreHandle_t g_pressEventLab11;

}  // namespace Lab11
