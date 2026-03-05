/*
 *  TaskButtonMeasure.h
 *  -------------------
 *  Detects button press / release and measures its duration.
 *  Drives green (short) or red (long) LED on release.
 */

#pragma once
#include <Arduino.h>

/*  Call once before the scheduler is started.
 *  Stores the three pin numbers used by the task.          */
void TaskButtonMeasureInit(uint8_t buttonPin,
                           uint8_t greenLedPin,
                           uint8_t redLedPin);

/*  Scheduler callback — period 20 ms, offset 0 ms.
 *  Must be registered before TaskPressStats so it
 *  produces before the consumer runs.                      */
void TaskButtonMeasure();
