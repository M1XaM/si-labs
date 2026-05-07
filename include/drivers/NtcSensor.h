#pragma once

#include <Arduino.h>

namespace NtcSensor {

constexpr float ReferenceResistance = 10000.0f;
constexpr float ReferenceKelvin = 298.15f;
constexpr float Beta = 3950.0f;
constexpr float DividerResistance = 10000.0f;

void begin(uint8_t analogPin);
uint16_t readRaw(uint8_t analogPin);
float toCelsius(uint16_t adcValue);
int16_t toCelsius10(uint16_t adcValue);

} // namespace NtcSensor
