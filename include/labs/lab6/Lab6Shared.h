#pragma once

#include <stdint.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

struct SensorDataLab6 {
    uint32_t rawValue;
    uint32_t saturatedValue;
    uint32_t medianFilteredValue;
    float weightedAvgValue;
};

extern SensorDataLab6 g_sensorDataLab6;
extern SemaphoreHandle_t g_sensorDataMutexLab6;

// Shared configuration
#define SENSOR_PIN A0

// Conditioning Params
#define SATURATION_MIN 100
#define SATURATION_MAX 900
#define MEDIAN_WINDOW_SIZE 5
#define WEIGHT_ALPHA 0.2f // New value weight. 0.2 means 80% history, 20% new. Slow response.
