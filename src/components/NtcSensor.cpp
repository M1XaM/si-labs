#include "drivers/NtcSensor.h"

#include <math.h>

namespace NtcSensor {

void begin(uint8_t analogPin) {
    pinMode(analogPin, INPUT);
}

uint16_t readRaw(uint8_t analogPin) {
    return (uint16_t)analogRead(analogPin);
}

float toCelsius(uint16_t adcValue) {
    if (adcValue == 0) {
        adcValue = 1;
    } else if (adcValue >= 1023) {
        adcValue = 1022;
    }

    const float adc = (float)adcValue;
    const float resistance = DividerResistance * adc / (1023.0f - adc);
    const float invKelvin = (1.0f / ReferenceKelvin) +
                            (log(resistance / ReferenceResistance) / Beta);
    return (1.0f / invKelvin) - 273.15f;
}

int16_t toCelsius10(uint16_t adcValue) {
    return (int16_t)(toCelsius(adcValue) * 10.0f);
}

} // namespace NtcSensor
