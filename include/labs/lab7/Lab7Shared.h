#pragma once

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

namespace Lab7 {

constexpr uint8_t RelayPin = 9;
constexpr uint8_t GreenPin = 12;
constexpr uint8_t RedPin = 11;
constexpr uint8_t YellowPin = 10;

constexpr uint8_t RowPins[] = {2, 3, 4, 5};
constexpr uint8_t ColPins[] = {A0, A1, A2, A3};

constexpr char OnKey = '1';
constexpr char OnAltKey = 'A';
constexpr char OffKey = '0';
constexpr char OffAltKey = 'B';

constexpr uint8_t StableSamples = 5;
constexpr uint16_t ReadPeriodMs = 50;
constexpr uint16_t FilterPeriodMs = 50;
constexpr uint16_t DrivePeriodMs = 100;
constexpr uint16_t ReportPeriodMs = 500;

struct BinaryActuatorState {
    bool requested;
    bool requestSeen;
    bool accepted;
    bool driven;
    bool fastChangeWarning;
    uint8_t debounceProgress;
};

extern BinaryActuatorState State;
extern SemaphoreHandle_t StateLock;
extern SemaphoreHandle_t CommandSignal;

} // namespace Lab7
