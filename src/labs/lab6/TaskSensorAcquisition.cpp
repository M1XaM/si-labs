#include "labs/lab6/TaskSensorAcquisition.h"
#include "labs/lab6/Lab6Shared.h"
#include <Arduino.h>

void TaskSensorAcquisitionLab6(void *pvParameters) {
    (void) pvParameters;

    pinMode(SENSOR_PIN, INPUT);

    while (1) {
        // Read raw data
        uint32_t raw = analogRead(SENSOR_PIN);
        
        // Update shared structure
        if (xSemaphoreTake(g_sensorDataMutexLab6, portMAX_DELAY)) {
            g_sensorDataLab6.rawValue = raw;
            xSemaphoreGive(g_sensorDataMutexLab6);
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Acquisition rate 50ms (20Hz)
    }
}
