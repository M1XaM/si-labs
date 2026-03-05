/*
 *  TaskPressStats.h
 *  ----------------
 *  Consumes press results, accumulates statistics, and
 *  drives the yellow LED blink pattern.
 */

#pragma once
#include <Arduino.h>

/*  Store the yellow-LED pin and reset internal blink state.  */
void TaskPressStatsInit(uint8_t yellowLedPin);

/*  Scheduler callback — period 20 ms, offset 0 ms.
 *  Must be registered after TaskButtonMeasure.               */
void TaskPressStats();
