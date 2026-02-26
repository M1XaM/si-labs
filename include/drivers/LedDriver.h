#ifndef _LED_DRV_H_
#define _LED_DRV_H_

#include <Arduino.h>

/* Configure a single GPIO as digital output for an LED */
void InitializeLed(int gpioPin);

/* Drive an LED on or off */
void SetLedState(int gpioPin, bool turnOn);

#endif /* _LED_DRV_H_ */
