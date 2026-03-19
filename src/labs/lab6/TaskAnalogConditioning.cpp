#include "labs/lab6/TaskAnalogConditioning.h"
#include "labs/lab6/Lab6Shared.h"
#include <Arduino.h>

// Simple bubble sort if std::sort not available or to reduce dependencies
void bubbleSort(uint32_t* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                uint32_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void TaskAnalogConditioningLab6(void *pvParameters) {
    (void) pvParameters;

    // Median Filter State
    uint32_t medianBuffer[MEDIAN_WINDOW_SIZE] = {0};
    int medianIndex = 0;
    bool bufferFilled = false;

    // Weighted Average State
    float currentWeightedAvg = 0.0f;
    bool firstSample = true;

    while (1) {
        uint32_t raw = 0;

        // 1. Read Raw Value
        if (xSemaphoreTake(g_sensorDataMutexLab6, portMAX_DELAY)) {
            raw = g_sensorDataLab6.rawValue;
            xSemaphoreGive(g_sensorDataMutexLab6);
        }

        // 2. Saturation
        uint32_t saturated = raw;
        if (saturated < SATURATION_MIN) saturated = SATURATION_MIN;
        if (saturated > SATURATION_MAX) saturated = SATURATION_MAX;

        // 3. Median Filter
        if (medianIndex == 0 && !bufferFilled) {
             // Pre-fill on first run to avoid zeroes affecting median
             for(int k=0; k<MEDIAN_WINDOW_SIZE; k++) medianBuffer[k] = raw;
             bufferFilled = true;
        }

        medianBuffer[medianIndex] = raw;
        medianIndex = (medianIndex + 1) % MEDIAN_WINDOW_SIZE;

        uint32_t tempBuffer[MEDIAN_WINDOW_SIZE];
        for (int i = 0; i < MEDIAN_WINDOW_SIZE; i++) {
            tempBuffer[i] = medianBuffer[i];
        }
        
        // Sort
        bubbleSort(tempBuffer, MEDIAN_WINDOW_SIZE);
        uint32_t median = tempBuffer[MEDIAN_WINDOW_SIZE / 2];

        // 4. Weighted Average
        if (firstSample) {
            currentWeightedAvg = (float)raw;
            firstSample = false;
        } else {
            currentWeightedAvg = (WEIGHT_ALPHA * (float)raw) + 
                                 ((1.0f - WEIGHT_ALPHA) * currentWeightedAvg);
        }

        // Update Shared Structure
        if (xSemaphoreTake(g_sensorDataMutexLab6, portMAX_DELAY)) {
            g_sensorDataLab6.saturatedValue = saturated;
            g_sensorDataLab6.medianFilteredValue = median;
            g_sensorDataLab6.weightedAvgValue = currentWeightedAvg;
            xSemaphoreGive(g_sensorDataMutexLab6);
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Run at same rate as acquisition or similar
    }
}
