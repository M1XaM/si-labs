#include "labs/lab11/TaskButtonRead11.h"

#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "drivers/ButtonDriver.h"
#include "labs/lab11/Lab11Shared.h"

// ── Button sampling with temporal debounce ───────────────────────────────────
// Polls kButtonPin at kButtonPeriodMs. A transition is confirmed only after
// kDebounceSamples consecutive identical raw readings, then one binary
// semaphore is given for every HIGH→LOW edge (i.e. one per physical press).
void TaskButtonRead11Func(void* pvParameters) {
    using namespace Lab11;

    const TickType_t xPeriod = pdMS_TO_TICKS(kButtonPeriodMs);
    TickType_t xLastWake = xTaskGetTickCount();

    bool    confirmedHigh  = true;   // idle = pull-up HIGH (released)
    bool    lastSample     = true;
    uint8_t stableCnt      = kDebounceSamples;

    for (;;) {
        const bool raw = ReadButtonRaw(kButtonPin);   // HIGH = released

        if (raw == lastSample) {
            if (stableCnt < kDebounceSamples) {
                ++stableCnt;
            }
        } else {
            lastSample = raw;
            stableCnt  = 1;
        }

        if (stableCnt >= kDebounceSamples) {
            const bool stableHigh = lastSample;

            // Rising edge? confirmed released → confirmed pressed
            if (confirmedHigh && !stableHigh) {
                xSemaphoreGive(g_pressEventLab11);
            }
            confirmedHigh = stableHigh;
        }

        vTaskDelayUntil(&xLastWake, xPeriod);
    }
}
