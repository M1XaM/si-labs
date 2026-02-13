#include "drivers/LedDriver.h"

void InitializeLed(int pinNumber) {
    pinMode(pinNumber, OUTPUT);
}

void SetLedState(int pinNumber, bool isTargetOn) {
    if (isTargetOn) {
        digitalWrite(pinNumber, HIGH);
    } else {
        digitalWrite(pinNumber, LOW);
    }
}