#pragma once

#include <stdint.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

struct SensorData {
    uint32_t rawValue;
    float temperature;
    bool isAlertActive;
    bool isStableAlert;
    uint32_t lastAlertChangeTime;
};

extern SensorData g_sensorData;
extern SemaphoreHandle_t g_sensorDataMutex;

#define TEMP_THRESHOLD_HIGH 25.0f
#define TEMP_HYSTERESIS      1.0f
#define STABLE_ALERT_TIME_MS 1000
