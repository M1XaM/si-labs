#include "labs/lab4/StatisticsTask.h"
#include "labs/lab4/Lab4Shared.h"
#include "drivers/LedDriver.h"

namespace StatisticsTask {
    void run(void* parameters) {
        
        while (true) {
            
            if (xSemaphoreTake(pressSemaphore, portMAX_DELAY) == pdTRUE) {
                uint32_t duration = ReadingData::lastDuration;

                xSemaphoreTake(statsMutex, portMAX_DELAY);

                if (duration < PRESS_DURATION_THRESHOLD_MS) {
                    StatisticsData::shortPressesNumber++;
                    StatisticsData::shortPressesTotalDuration += duration;
                } else {
                    StatisticsData::longPressesNumber++;
                    StatisticsData::longPressesTotalDuration += duration;
                }

                xSemaphoreGive(statsMutex);

                int blinkCount = (duration >= PRESS_DURATION_THRESHOLD_MS) ? LONG_BLINK_NUMBER : SHORT_BLINK_NUMBER;
                for (int i = 0; i < blinkCount; i++) {
                    SetLedState(PIN_LED_Y, true);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    SetLedState(PIN_LED_Y, false);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
        }
    }
}
