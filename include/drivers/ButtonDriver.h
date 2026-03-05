#pragma once
#include <Arduino.h>

// Initializes a button pin with INPUT_PULLUP.
void InitializeButton(uint8_t pin);

// Returns true when the button is pressed (active-LOW with pull-up).
bool ReadButtonRaw(uint8_t pin);
