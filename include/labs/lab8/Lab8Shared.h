#pragma once

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

namespace Lab8 {

constexpr uint8_t ServoPin = 9;
constexpr uint8_t GreenPin = 12;
constexpr uint8_t RedPin = 11;
constexpr uint8_t YellowPin = 10;

constexpr uint8_t RowPins[] = {2, 3, 4, 5};
constexpr uint8_t ColPins[] = {A0, A1, A2, A3};

constexpr char CommitKey = '#';
constexpr char ClearKey = '*';
constexpr char StopKey = 'A';

constexpr uint8_t MinCommand = 0;
constexpr uint8_t MaxCommand = 100;
constexpr uint8_t MinAngle = 0;
constexpr uint8_t MaxAngle = 180;
constexpr uint8_t MedianWindow = 5;
constexpr uint8_t AverageWindow = 4;
constexpr uint8_t RampStep = 2;
constexpr uint8_t InputBufferSize = 8;

constexpr uint16_t ReadPeriodMs = 50;
constexpr uint16_t DrivePeriodMs = 50;
constexpr uint16_t ReportPeriodMs = 500;

struct AnalogActuatorState {
    uint8_t rawCommand;
    uint8_t saturated;
    uint8_t median;
    uint8_t weighted;
    uint8_t rampTarget;
    uint8_t current;
    uint8_t angle;
    bool clipped;
};

extern AnalogActuatorState State;
extern SemaphoreHandle_t StateLock;
extern SemaphoreHandle_t CommandReady;

uint8_t clampPercent(uint8_t value);
uint8_t medianOf(uint8_t* values, uint8_t count);
uint8_t weightedNewestFirst(const uint8_t* values, uint8_t count);
uint8_t approach(uint8_t current, uint8_t target, uint8_t step);

} // namespace Lab8
