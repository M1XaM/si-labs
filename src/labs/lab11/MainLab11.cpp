#include "labs/lab11/MainLab11.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdio.h>

#include "drivers/ButtonDriver.h"
#include "drivers/LedDriver.h"
#include "drivers/SerialStream.h"
#include "labs/lab11/Lab11Shared.h"
#include "labs/lab11/TaskButtonRead11.h"
#include "labs/lab11/TaskFsmProcess11.h"
#include "labs/lab11/TaskLedActuator11.h"
#include "labs/lab11/TaskSerialDisplay11.h"
#include "services/StdioRedirect.h"

namespace Lab11 {

// ── Global state ────────────────────────────────────────────────────────────
volatile FsmStateLab11 g_ledStateLab11   = FsmStateLab11::Off;
volatile uint32_t      g_pressCountLab11 = 0;

SemaphoreHandle_t g_stateMutexLab11 = nullptr;
SemaphoreHandle_t g_pressEventLab11 = nullptr;

// ── FSM logic (pure) ────────────────────────────────────────────────────────
FsmStateLab11 stepFsmLab11(FsmStateLab11 current, FsmEventLab11 event) {
    if (event == FsmEventLab11::Press) {
        return (current == FsmStateLab11::Off) ? FsmStateLab11::On
                                               : FsmStateLab11::Off;
    }
    return current;
}

const char* fsmLabelLab11(FsmStateLab11 state) {
    return (state == FsmStateLab11::On) ? "ON" : "OFF";
}

}  // namespace Lab11

static SerialStream sio;

void SetupLab11() {
    using namespace Lab11;

    sio.begin(9600);
    initStdio(&sio);

    InitializeButton(kButtonPin);

    InitializeLed(kLedGreenPin);
    InitializeLed(kLedRedPin);

    // Initial LED pattern: green ON (Off state), red OFF
    SetLedState(kLedGreenPin,  true);
    SetLedState(kLedRedPin,    false);

    g_stateMutexLab11 = xSemaphoreCreateMutex();
    g_pressEventLab11 = xSemaphoreCreateBinary();

    xTaskCreate(TaskButtonRead11Func,   "Btn",   128, nullptr, 3, nullptr);
    xTaskCreate(TaskFsmProcess11Func,   "Fsm",   128, nullptr, 3, nullptr);
    xTaskCreate(TaskLedActuator11Func,  "Act",   128, nullptr, 2, nullptr);
    xTaskCreate(TaskSerialDisplay11Func,"Disp",  192, nullptr, 1, nullptr);

    printf_P(PSTR("Lab 11: FSM Button LED Toggle (Part 7.2.1)\n"));
    printf_P(PSTR("Btn:D%u  Grn(Off):D%u  Red(On):D%u\n"),
        (unsigned)kButtonPin, (unsigned)kLedGreenPin,
        (unsigned)kLedRedPin);
    printf_P(PSTR("Debounce: %u shots @ %u ms\n"),
        (unsigned)kDebounceSamples, (unsigned)kButtonPeriodMs);

    vTaskStartScheduler();
}

void LoopLab11() {
}
