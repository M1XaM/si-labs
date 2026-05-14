#include "labs/lab11/TaskFsmProcess11.h"

#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "labs/lab11/Lab11Shared.h"

// ── Event-driven FSM runner ──────────────────────────────────────────────────
// Suspends until the button task posts a press event (binary semaphore),
// then atomically advances the FSM and increments the lifetime press counter.
void TaskFsmProcess11Func(void* pvParameters) {
    using namespace Lab11;

    for (;;) {
        if (xSemaphoreTake(g_pressEventLab11, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        xSemaphoreTake(g_stateMutexLab11, portMAX_DELAY);
        {
            g_ledStateLab11 = stepFsmLab11(
                (FsmStateLab11)g_ledStateLab11,
                FsmEventLab11::Press);
            ++g_pressCountLab11;
        }
        xSemaphoreGive(g_stateMutexLab11);
    }
}
