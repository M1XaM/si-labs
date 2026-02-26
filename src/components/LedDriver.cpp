#include "drivers/LedDriver.h"

/* Configure a GPIO pin as digital output for an LED */
void InitializeLed(int gpioPin)
{
    pinMode(gpioPin, OUTPUT);
}

/* Drive the LED HIGH (on) or LOW (off) */
void SetLedState(int gpioPin, bool turnOn)
{
    digitalWrite(gpioPin, turnOn ? HIGH : LOW);
}