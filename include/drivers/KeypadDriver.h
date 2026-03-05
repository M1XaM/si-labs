#pragma once
#include <Arduino.h>

const uint8_t KeypadRowCount = 4;
const uint8_t KeypadColCount = 4;

const uint16_t KeypadDebounceMs = 50;
const uint16_t KeypadReleaseMs  = 20;

void InitializeKeypad(const uint8_t rowPins[KeypadRowCount],
                      const uint8_t colPins[KeypadColCount]);

bool IsKeypadKeyAvailable();
char ScanKeypad();
