#include "drivers/ButtonDriver.h"

void InitializeButton(uint8_t pin) {
    pinMode(pin, INPUT_PULLUP);
}

bool ReadButtonRaw(uint8_t pin) {
    return digitalRead(pin) == HIGH;  // active-LOW: pulled up, shorts to GND when pressed
}
