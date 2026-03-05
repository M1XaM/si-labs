#pragma once
#include <Arduino.h>

void InitializeLed(int pinNumber);

void SetLedState(int pinNumber, bool isTargetOn);
