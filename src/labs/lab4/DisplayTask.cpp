#include "labs/lab4/DisplayTask.h"
#include "labs/lab4/Lab4Shared.h"
#include <stdio.h>

namespace DisplayTask {
    void run(void* parameters) {
        TickType_t lastWakeTime = xTaskGetTickCount();
        const TickType_t interval = pdMS_TO_TICKS(10 * 1000); // 10 seconds

        while (true) {
            vTaskDelayUntil(&lastWakeTime, interval);

            xSemaphoreTake(statsMutex, portMAX_DELAY);

            uint16_t currentShort = StatisticsData::shortPressesNumber;
            uint16_t currentLong = StatisticsData::longPressesNumber;
            unsigned long currentShortDuration = StatisticsData::shortPressesTotalDuration;
            unsigned long currentLongDuration = StatisticsData::longPressesTotalDuration;

            // Reset stats
            StatisticsData::shortPressesNumber = 0;
            StatisticsData::longPressesNumber = 0;
            StatisticsData::shortPressesTotalDuration = 0;
            StatisticsData::longPressesTotalDuration = 0;

            xSemaphoreGive(statsMutex);

            unsigned long totalCount = currentShort + currentLong;
            unsigned long totalDuration = currentShortDuration + currentLongDuration;

            if (totalCount > 0) {
                unsigned long avgMs = totalDuration / totalCount;
                unsigned long wholeSeconds = avgMs / 1000;
                unsigned long fractionalSeconds = avgMs % 1000;
                unsigned long decimals = fractionalSeconds / 10;

                printf_P(PSTR("L: %u, S: %u, Avg: %lu.%02lus\n\r"),
                    currentLong,
                    currentShort,
                    wholeSeconds,
                    decimals);
            } else {
                printf_P(PSTR("L: %u, S: %u, Avg: 0.00s\n\r"),
                    currentLong,
                    currentShort);
            }
        }
    }
}
