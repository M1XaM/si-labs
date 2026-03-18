#include "labs/lab4/ReadingTask.h"
#include "labs/lab4/Lab4Shared.h"
#include "drivers/ButtonDriver.h"
#include "drivers/LedDriver.h"

namespace ReadingTask {
    void visualizeButtonPressDuration(uint32_t duration);

    void run(void* parameters) {
        // Init happens in MainLab4 before scheduler or here if local.
        // But pins are inited in MainLab4 usually.

        uint32_t startTime = 0;
        bool lastState = false;

        while (true) {
            // ReadButtonRaw returns true if pressed (LOW)
            bool isPressed = ReadButtonRaw(PIN_BTN);

            // Button just pressed
            if (isPressed && !lastState) {
                startTime = millis();
            }

            // Button just released
            if (!isPressed && lastState && startTime != 0) {
                uint32_t duration = millis() - startTime;
                ReadingData::lastDuration = duration;
                startTime = 0;

                xSemaphoreGive(pressSemaphore);
                visualizeButtonPressDuration(duration);
            }

            lastState = isPressed;
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }

    void visualizeButtonPressDuration(uint32_t duration) {
        if (duration < PRESS_DURATION_THRESHOLD_MS) {
            SetLedState(PIN_LED_G, true);
            SetLedState(PIN_LED_R, false);
        } else {
            SetLedState(PIN_LED_R, true);
            SetLedState(PIN_LED_G, false);
        }
    }
}
