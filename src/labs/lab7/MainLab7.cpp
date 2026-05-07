#include "labs/lab7/MainLab7.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <stdio.h>

#include "drivers/KeypadDriver.h"
#include "drivers/LedDriver.h"
#include "drivers/SerialStream.h"
#include "labs/lab7/Lab7Shared.h"
#include "services/StdioRedirect.h"

namespace Lab7 {

BinaryActuatorState State = {false, false, false, false, false, 0};
SemaphoreHandle_t StateLock = nullptr;
SemaphoreHandle_t CommandSignal = nullptr;

static SerialStream Console;

static void publishRequest(bool nextValue) {
    xSemaphoreTake(StateLock, portMAX_DELAY);
    State.requested = nextValue;
    State.requestSeen = true;
    xSemaphoreGive(StateLock);
    xSemaphoreGive(CommandSignal);
}

static bool decodeKey(char key, bool* value) {
    if (key == OnKey || key == OnAltKey) {
        *value = true;
        return true;
    }
    if (key == OffKey || key == OffAltKey) {
        *value = false;
        return true;
    }
    return false;
}

static void TaskReadCommand(void*) {
    const TickType_t period = pdMS_TO_TICKS(ReadPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        if (IsKeypadKeyAvailable()) {
            bool decodedValue = false;
            if (decodeKey(ScanKeypad(), &decodedValue)) {
                publishRequest(decodedValue);
            }
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskDebounceCommand(void*) {
    const TickType_t period = pdMS_TO_TICKS(FilterPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();
    bool output = false;
    bool previousSample = false;
    uint8_t sameSampleCount = 0;

    for (;;) {
        xSemaphoreTake(StateLock, portMAX_DELAY);
        const bool sample = State.requested;
        xSemaphoreGive(StateLock);

        if (sample == output) {
            sameSampleCount = 0;
        } else {
            sameSampleCount = (sample == previousSample) ? sameSampleCount + 1 : 1;
            if (sameSampleCount >= StableSamples) {
                output = sample;
                sameSampleCount = 0;
            }
        }
        previousSample = sample;

        xSemaphoreTake(StateLock, portMAX_DELAY);
        State.accepted = output;
        State.debounceProgress = sameSampleCount;
        xSemaphoreGive(StateLock);

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskDriveActuator(void*) {
    const TickType_t period = pdMS_TO_TICKS(DrivePeriodMs);
    TickType_t lastWake = xTaskGetTickCount();
    bool activity = false;

    for (;;) {
        xSemaphoreTake(StateLock, portMAX_DELAY);
        const bool desired = State.accepted;
        xSemaphoreGive(StateLock);

        SetLedState(RelayPin, desired);
        SetLedState(GreenPin, !desired);
        SetLedState(RedPin, desired);

        activity = !activity;
        SetLedState(YellowPin, activity);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        State.driven = desired;
        xSemaphoreGive(StateLock);

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskReportState(void*) {
    const TickType_t period = pdMS_TO_TICKS(ReportPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, period);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        const BinaryActuatorState snapshot = State;
        xSemaphoreGive(StateLock);

        printf_P(PSTR("REQ:%s READY:%s DB:%u/%u OUT:%s %s\n"),
                 snapshot.requested ? "ON" : "OFF",
                 snapshot.accepted ? "ON" : "OFF",
                 snapshot.debounceProgress,
                 StableSamples,
                 snapshot.driven ? "ON" : "OFF",
                 snapshot.fastChangeWarning ? "WARN" : "OK");
    }
}

} // namespace Lab7

void SetupLab7() {
    Lab7::Console.begin(9600);
    initStdio(&Lab7::Console);

    InitializeLed(Lab7::RelayPin);
    InitializeLed(Lab7::GreenPin);
    InitializeLed(Lab7::RedPin);
    InitializeLed(Lab7::YellowPin);
    InitializeKeypad(Lab7::RowPins, Lab7::ColPins);

    SetLedState(Lab7::RelayPin, false);
    SetLedState(Lab7::GreenPin, true);
    SetLedState(Lab7::RedPin, false);
    SetLedState(Lab7::YellowPin, false);

    Lab7::StateLock = xSemaphoreCreateMutex();
    Lab7::CommandSignal = xSemaphoreCreateBinary();

    xTaskCreate(Lab7::TaskReadCommand, "L7Read", 256, nullptr, 3, nullptr);
    xTaskCreate(Lab7::TaskDebounceCommand, "L7Deb", 128, nullptr, 2, nullptr);
    xTaskCreate(Lab7::TaskDriveActuator, "L7Drv", 128, nullptr, 2, nullptr);
    xTaskCreate(Lab7::TaskReportState, "L7Rep", 256, nullptr, 1, nullptr);

    printf_P(PSTR("Lab 7: binary actuator control\n"));
    printf_P(PSTR("Keys: %c/%c=ON, %c/%c=OFF, debounce=%u samples\n"),
             Lab7::OnKey,
             Lab7::OnAltKey,
             Lab7::OffKey,
             Lab7::OffAltKey,
             Lab7::StableSamples);

    vTaskStartScheduler();
}

void LoopLab7() {
}
