#pragma once

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

namespace Lab10 {

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

constexpr int32_t DefaultKpQ88 = (int32_t)(4.0f * 256);
constexpr int32_t DefaultKiQ88 = (int32_t)(0.10f * 256);
constexpr int32_t DefaultKdQ88 = (int32_t)(0.50f * 256);
constexpr int32_t IntegralLimit = 5000;
constexpr int16_t OutputLimit = 100;
constexpr uint8_t ServoCenter = 90;
constexpr uint8_t ServoSwing = 90;

constexpr uint16_t AcquisitionPeriodMs = 50;
constexpr uint16_t CommandPeriodMs = 50;
constexpr uint16_t ControlPeriodMs = 100;
constexpr uint16_t ActuatorPeriodMs = 50;
constexpr uint16_t ReportPeriodMs = 500;
constexpr uint8_t InputBufferSize = 16;

struct PidState {
    uint16_t adc;
    int16_t temperatureC10;
    int16_t setPointC10;
    int32_t kpQ88;
    int32_t kiQ88;
    int32_t kdQ88;
    int32_t integral;
    int16_t previousError;
    int16_t output;
    uint8_t angle;
};

extern PidState State;
extern SemaphoreHandle_t StateLock;

int16_t clampSetPoint(int16_t value);
int16_t calculatePid(int16_t setPoint,
                     int16_t measured,
                     int32_t kpQ88,
                     int32_t kiQ88,
                     int32_t kdQ88,
                     int32_t* integral,
                     int16_t* previousError);
bool parseQ88(const char* text, int32_t* result);

} // namespace Lab10
