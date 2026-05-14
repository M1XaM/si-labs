#include "labs/lab11/TaskLedActuator11.h"

#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "drivers/LedDriver.h"
#include "labs/lab11/Lab11Shared.h"

// ── LED actuator ─────────────────────────────────────────────────────────────
// Reads the shared FSM state and drives the physical LEDs:
//   • Green = ON  when FSM is Off  (standby indicator)
//   • Red   = ON  when FSM is On   (active indicator)
void TaskLedActuator11Func(void* pvParameters) {
    using namespace Lab11;

    const TickType_t xPeriod = pdMS_TO_TICKS(kActuatorPeriodMs);
    TickType_t xLastWake = xTaskGetTickCount();

    for (;;) {
        xSemaphoreTake(g_stateMutexLab11, portMAX_DELAY);
        const bool fsmOn = (FsmStateLab11)g_ledStateLab11 == FsmStateLab11::On;
        xSemaphoreGive(g_stateMutexLab11);

        SetLedState(kLedGreenPin,  !fsmOn);
        SetLedState(kLedRedPin,     fsmOn);

        vTaskDelayUntil(&xLastWake, xPeriod);
    }
}
