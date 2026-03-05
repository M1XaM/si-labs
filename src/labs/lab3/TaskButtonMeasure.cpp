#include "labs/lab3/TaskButtonMeasure.h"

#include <Arduino.h>

#include "drivers/ButtonDriver.h"
#include "drivers/LedDriver.h"
#include "services/PressData.h"

/* ---- cached pin numbers (set once in Init) ---- */
static uint8_t pinBtn   = 0;
static uint8_t pinGreen = 0;
static uint8_t pinRed   = 0;

/* ---- FSM state ---- */
typedef enum { ST_IDLE, ST_PRESSED } MeasureState_t;
static MeasureState_t fsm       = ST_IDLE;
static uint32_t       stampMs   = 0;

/* ================================================ */

void TaskButtonMeasureInit(uint8_t buttonPin,
                           uint8_t greenLedPin,
                           uint8_t redLedPin)
{
    pinBtn   = buttonPin;
    pinGreen = greenLedPin;
    pinRed   = redLedPin;
    fsm      = ST_IDLE;
}

/* ------------------------------------------------ */

void TaskButtonMeasure()
{
    switch (fsm)
    {
    case ST_IDLE:
    {
        if (!ReadButtonRaw(pinBtn))
            break;

        /* new press detected — clear previous result LEDs */
        SetLedState(pinGreen, false);
        SetLedState(pinRed,   false);
        stampMs = millis();
        fsm     = ST_PRESSED;
        break;
    }

    case ST_PRESSED:
    {
        if (ReadButtonRaw(pinBtn))
            break;                       /* still held — wait */

        /* released — compute duration and publish */
        uint32_t dur = millis() - stampMs;
        SetPressResult(dur);

        if (dur < ShortPressThresholdMs)
            SetLedState(pinGreen, true);
        else
            SetLedState(pinRed, true);

        fsm = ST_IDLE;
        break;
    }
    }
}
