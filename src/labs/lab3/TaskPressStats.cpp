#include "labs/lab3/TaskPressStats.h"

#include <Arduino.h>

#include "drivers/LedDriver.h"
#include "services/PressData.h"

/* ---- pin cache ---- */
static uint8_t pinYellow = 0;

/* ---- blink engine constants ----
 * One phase = PHASE_TICKS × 20 ms = 100 ms.
 */
static const uint8_t PHASE_TICKS = 5;

/* ---- blink engine state ---- */
static uint8_t blinksLeft = 0;
static uint8_t tickCnt    = 0;
static bool    ledOn      = false;

/* ------------------------------------------------
 * resetBlink — arm a new blink sequence
 * ------------------------------------------------ */
static void resetBlink(uint8_t count)
{
    blinksLeft = count;
    tickCnt    = 0;
    ledOn      = false;
    SetLedState(pinYellow, false);
}

/* ------------------------------------------------
 * stepBlink — advance the blink FSM by one tick
 * ------------------------------------------------ */
static void stepBlink()
{
    if (blinksLeft == 0)
        return;

    if (tickCnt > 0) {
        --tickCnt;
        return;
    }

    if (!ledOn) {
        SetLedState(pinYellow, true);
        ledOn   = true;
        tickCnt = PHASE_TICKS - 1;
    } else {
        SetLedState(pinYellow, false);
        ledOn = false;
        --blinksLeft;
        tickCnt = PHASE_TICKS - 1;
    }
}

/* ================================================ */

void TaskPressStatsInit(uint8_t yellowLedPin)
{
    pinYellow = yellowLedPin;
    resetBlink(0);
}

void TaskPressStats()
{
    uint32_t dur;
    if (ConsumePressResult(&dur)) {
        RecordPress(dur);
        resetBlink(dur < ShortPressThresholdMs ? 5 : 10);
    }
    stepBlink();
}
