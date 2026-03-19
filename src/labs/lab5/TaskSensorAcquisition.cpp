#include "labs/lab5/TaskSensorAcquisition.h"
#include "labs/lab5/Lab5Shared.h"
#include <Arduino.h>
#include <math.h>

#define SENSOR_PIN A0 // Arduino Uno ADC pin
const float BETA = 3950.0f; // Beta Coefficient of the wokwi thermistor

void TaskSensorAcquisition(void* pvParameters) {
    while (1) {
        int rawValue = analogRead(SENSOR_PIN);
        
        // Calculate real temperature for standard Wokwi NTC 
        float temperature = 0.0f;
        if (rawValue > 0 && rawValue < 1023) {
            temperature = 1.0f / (log(1.0f / (1023.0f / rawValue - 1.0f)) / BETA + 1.0f / 298.15f) - 273.15f;
        }
        
        if (xSemaphoreTake(g_sensorDataMutex, portMAX_DELAY)) {
            g_sensorData.rawValue = rawValue;
            g_sensorData.temperature = temperature;
            xSemaphoreGive(g_sensorDataMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // requirement 20-100ms
    }
}
