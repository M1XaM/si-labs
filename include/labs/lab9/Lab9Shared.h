#pragma once

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

namespace Lab9 {

constexpr uint8_t SensorPin = A0;
constexpr uint8_t ServoPin = 9;
constexpr uint8_t GreenPin = 12;
constexpr uint8_t RedPin = 11;
constexpr uint8_t YellowPin = 10;

constexpr uint8_t RowPins[] = {2, 3, 4, 5};
constexpr uint8_t ColPins[] = {A1, A2, A3, A4};

constexpr char CommitKey = '#';
constexpr char ClearKey = '*';
constexpr char PlusOneKey = 'A';
constexpr char MinusOneKey = 'B';
constexpr char PlusTenKey = 'C';
constexpr char MinusTenKey = 'D';

constexpr int16_t DefaultSetPointC10 = 250;
constexpr int16_t MinSetPointC10 = -100;
constexpr int16_t MaxSetPointC10 = 500;
constexpr int16_t HysteresisC10 = 10;

constexpr uint8_t HeatAngle = 180;
constexpr uint8_t CoolAngle = 0;
constexpr uint8_t NeutralAngle = 90;

constexpr uint16_t AcquisitionPeriodMs = 50;
constexpr uint16_t CommandPeriodMs = 50;
constexpr uint16_t ControlPeriodMs = 50;
constexpr uint16_t ActuatorPeriodMs = 50;
constexpr uint16_t ReportPeriodMs = 500;
constexpr uint8_t InputBufferSize = 8;

struct ControlState {
    uint16_t adc;
    int16_t temperatureC10;
    int16_t setPointC10;
    bool outputOn;
    uint8_t angle;
};

extern ControlState State;
extern SemaphoreHandle_t StateLock;

int16_t clampSetPoint(int16_t value);

} // namespace Lab9
