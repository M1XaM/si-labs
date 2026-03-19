#include "labs/lab5/TaskThresholdAlerting.h"
#include "labs/lab5/Lab5Shared.h"
#include <Arduino.h>

#define ALERT_LED_PIN 11

void TaskThresholdAlerting(void* pvParameters) {
    bool localAlertState = false;
    uint32_t lastStateChange = millis();

    while (1) {
        if (xSemaphoreTake(g_sensorDataMutex, portMAX_DELAY)) {
            float temp = g_sensorData.temperature;
            
            // Threshold Check with Hysteresis
            bool newState = localAlertState;
            if (!localAlertState && temp >= TEMP_THRESHOLD_HIGH) {
                newState = true;
            } else if (localAlertState && temp <= (TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS)) {
                newState = false;
            }
            
            // Update tracking variables only when state actually transitions
            if (newState != localAlertState) {
                localAlertState = newState;
                lastStateChange = millis();
                g_sensorData.isAlertActive = localAlertState;
                g_sensorData.lastAlertChangeTime = lastStateChange;
            }
            
            // Apply antibouncing rule: lock into stable alert only after time elapsed
            uint32_t currentMillis = millis();
            if ((currentMillis - lastStateChange) >= STABLE_ALERT_TIME_MS) {
                g_sensorData.isStableAlert = localAlertState;
            }
            
            // Drive hardware LED directly reflecting the confirmed stable alert
            digitalWrite(ALERT_LED_PIN, g_sensorData.isStableAlert ? HIGH : LOW);
            
            xSemaphoreGive(g_sensorDataMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // check every 100ms
    }
}
