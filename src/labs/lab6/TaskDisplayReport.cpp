#include "labs/lab6/TaskDisplayReport.h"
#include "labs/lab6/Lab6Shared.h"
#include <Arduino.h>
#include <stdio.h>

void TaskDisplayReportLab6(void *pvParameters) {
    (void) pvParameters;

    while (1) {
        if (xSemaphoreTake(g_sensorDataMutexLab6, portMAX_DELAY)) {
            // Split float into integer and fractional parts for integer-only printf
            float avg = g_sensorDataLab6.weightedAvgValue;
            int avgInt = (int)avg;
            int avgFrac = abs((int)((avg - avgInt) * 100)); // 2 decimal places

            printf("[Lab6] Raw: %lu | Sat: %lu | Med: %lu | Avg: %d.%02d\n", 
                   g_sensorDataLab6.rawValue, 
                   g_sensorDataLab6.saturatedValue, 
                   g_sensorDataLab6.medianFilteredValue, 
                   avgInt, avgFrac);
            xSemaphoreGive(g_sensorDataMutexLab6);
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // Report every 500ms
    }
}
