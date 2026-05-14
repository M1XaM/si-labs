#include "labs/lab11/TaskSerialDisplay11.h"

#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdio.h>

#include "labs/lab11/Lab11Shared.h"

// ── Serial reporter ──────────────────────────────────────────────────────────
// Every kDisplayPeriodMs prints:
//   1) A plotter-friendly line:  STATE:{0|1}<TAB>PRESSES:{count}
//   2) A human-readable line:   # LED=ON|OFF  presses={count}
void TaskSerialDisplay11Func(void* pvParameters) {
    using namespace Lab11;

    const TickType_t xPeriod = pdMS_TO_TICKS(kDisplayPeriodMs);
    TickType_t xLastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&xLastWake, xPeriod);

        xSemaphoreTake(g_stateMutexLab11, portMAX_DELAY);
        const bool fsmOn = (FsmStateLab11)g_ledStateLab11 == FsmStateLab11::On;
        const uint32_t cnt = g_pressCountLab11;
        xSemaphoreGive(g_stateMutexLab11);

        printf_P(PSTR("STATE:%u\tPRESSES:%lu\n"),
            (unsigned)(fsmOn ? 1 : 0),
            (unsigned long)cnt);

        printf_P(PSTR("# LED=%s  presses=%lu\n"),
            fsmLabelLab11(fsmOn ? FsmStateLab11::On : FsmStateLab11::Off),
            (unsigned long)cnt);
    }
}
