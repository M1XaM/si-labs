#include "labs/lab5/TaskDisplayReport.h"
#include "labs/lab5/Lab5Shared.h"
#include <Arduino.h>
#include <stdio.h>

void TaskDisplayReport(void* pvParameters) {
    while (1) {
        if (xSemaphoreTake(g_sensorDataMutex, portMAX_DELAY)) {
            float temp = g_sensorData.temperature;
            bool isStable = g_sensorData.isStableAlert;
            
            // AVR printf doesn't support %f by default, cast to int for display or use dtostrf
            printf("Temp: %d *C | LED: %s\n", (int)temp, isStable ? "ON" : "OFF");
                
            xSemaphoreGive(g_sensorDataMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500)); // 500ms report interval as per PDF
    }
}
