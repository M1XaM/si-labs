#include "labs/lab8/MainLab8.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Servo.h>
#include <semphr.h>
#include <stdio.h>
#include <stdlib.h>
#include <task.h>

#include "drivers/KeypadDriver.h"
#include "drivers/LedDriver.h"
#include "drivers/SerialStream.h"
#include "labs/lab8/Lab8Shared.h"
#include "services/StdioRedirect.h"

namespace Lab8 {

AnalogActuatorState State = {0, 0, 0, 0, 0, 0, 0, false};
SemaphoreHandle_t StateLock = nullptr;
SemaphoreHandle_t CommandReady = nullptr;

static const uint16_t AverageWeights[AverageWindow] = {50, 25, 15, 10};
static SerialStream Console;
static Servo OutputServo;

uint8_t clampPercent(uint8_t value) {
    return value > MaxCommand ? MaxCommand : value;
}

uint8_t medianOf(uint8_t* values, uint8_t count) {
    for (uint8_t i = 1; i < count; ++i) {
        uint8_t x = values[i];
        int8_t j = i - 1;
        while (j >= 0 && values[j] > x) {
            values[j + 1] = values[j];
            --j;
        }
        values[j + 1] = x;
    }
    return values[count / 2];
}

uint8_t weightedNewestFirst(const uint8_t* values, uint8_t count) {
    uint32_t total = 0;
    uint32_t divisor = 0;
    for (uint8_t i = 0; i < count; ++i) {
        total += (uint32_t)values[i] * AverageWeights[i];
        divisor += AverageWeights[i];
    }
    return divisor == 0 ? 0 : (uint8_t)(total / divisor);
}

uint8_t approach(uint8_t current, uint8_t target, uint8_t step) {
    if (current < target) {
        const uint8_t delta = target - current;
        return current + (delta < step ? delta : step);
    }
    if (current > target) {
        const uint8_t delta = current - target;
        return current - (delta < step ? delta : step);
    }
    return current;
}

static void submitCommand(uint8_t value) {
    xSemaphoreTake(StateLock, portMAX_DELAY);
    State.rawCommand = value;
    xSemaphoreGive(StateLock);
    xSemaphoreGive(CommandReady);
}

static void flushNumber(char* buffer, uint8_t* length) {
    if (*length == 0) {
        return;
    }

    buffer[*length] = '\0';
    char* end = nullptr;
    long parsed = strtol(buffer, &end, 10);
    if (end != buffer && *end == '\0' && parsed >= 0) {
        submitCommand((uint8_t)(parsed > 255 ? 255 : parsed));
    }
    *length = 0;
}

static void TaskCollectCommand(void*) {
    const TickType_t period = pdMS_TO_TICKS(ReadPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();
    char serialBuffer[InputBufferSize];
    char keypadBuffer[InputBufferSize];
    uint8_t serialLength = 0;
    uint8_t keypadLength = 0;

    for (;;) {
        while (Serial.available() > 0) {
            char c = (char)Serial.read();
            if (c == '\n' || c == '\r') {
                flushNumber(serialBuffer, &serialLength);
            } else if (serialLength < InputBufferSize - 1) {
                serialBuffer[serialLength++] = c;
            }
        }

        while (IsKeypadKeyAvailable()) {
            const char key = ScanKeypad();
            if (key >= '0' && key <= '9') {
                if (keypadLength < InputBufferSize - 1) {
                    keypadBuffer[keypadLength++] = key;
                    putchar(key);
                }
            } else if (key == ClearKey) {
                keypadLength = 0;
                printf_P(PSTR(" [clear]\n"));
            } else if (key == StopKey) {
                keypadLength = 0;
                printf_P(PSTR(" [stop]\n"));
                submitCommand(0);
            } else if (key == CommitKey) {
                putchar('\n');
                flushNumber(keypadBuffer, &keypadLength);
            }
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskConditionCommand(void*) {
    uint8_t medianRing[MedianWindow] = {0};
    uint8_t averageRing[AverageWindow] = {0};
    uint8_t medianIndex = 0;
    uint8_t averageIndex = 0;
    uint8_t medianCount = 0;
    uint8_t averageCount = 0;
    bool activity = false;

    for (;;) {
        xSemaphoreTake(CommandReady, portMAX_DELAY);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        const uint8_t raw = State.rawCommand;
        xSemaphoreGive(StateLock);

        const uint8_t saturated = clampPercent(raw);
        medianRing[medianIndex] = saturated;
        medianIndex = (medianIndex + 1) % MedianWindow;
        if (medianCount < MedianWindow) {
            ++medianCount;
        }

        uint8_t medianWork[MedianWindow];
        for (uint8_t i = 0; i < medianCount; ++i) {
            medianWork[i] = medianRing[i];
        }
        const uint8_t median = medianOf(medianWork, medianCount);

        averageRing[averageIndex] = median;
        averageIndex = (averageIndex + 1) % AverageWindow;
        if (averageCount < AverageWindow) {
            ++averageCount;
        }

        uint8_t ordered[AverageWindow];
        for (uint8_t i = 0; i < averageCount; ++i) {
            const uint8_t source = (averageIndex + AverageWindow - 1 - i) % AverageWindow;
            ordered[i] = averageRing[source];
        }
        const uint8_t weighted = weightedNewestFirst(ordered, averageCount);

        activity = !activity;
        SetLedState(YellowPin, activity);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        State.saturated = saturated;
        State.median = median;
        State.weighted = weighted;
        State.rampTarget = weighted;
        State.clipped = raw > MaxCommand;
        xSemaphoreGive(StateLock);
    }
}

static void TaskDriveServo(void*) {
    const TickType_t period = pdMS_TO_TICKS(DrivePeriodMs);
    TickType_t lastWake = xTaskGetTickCount();
    uint8_t speed = 0;

    OutputServo.attach(ServoPin);
    OutputServo.write(MinAngle);

    for (;;) {
        xSemaphoreTake(StateLock, portMAX_DELAY);
        const uint8_t target = State.rampTarget;
        const bool clipped = State.clipped;
        xSemaphoreGive(StateLock);

        speed = approach(speed, target, RampStep);
        const uint8_t angle = (uint8_t)((uint16_t)speed * MaxAngle / MaxCommand);
        OutputServo.write(angle);
        SetLedState(GreenPin, !clipped);
        SetLedState(RedPin, clipped);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        State.current = speed;
        State.angle = angle;
        xSemaphoreGive(StateLock);

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskPrintReport(void*) {
    const TickType_t period = pdMS_TO_TICKS(ReportPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, period);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        const AnalogActuatorState snapshot = State;
        xSemaphoreGive(StateLock);

        printf_P(PSTR("CMD:%u SAT:%u MED:%u AVG:%u TARGET:%u OUT:%u%% ANG:%u %s\n"),
                 snapshot.rawCommand,
                 snapshot.saturated,
                 snapshot.median,
                 snapshot.weighted,
                 snapshot.rampTarget,
                 snapshot.current,
                 snapshot.angle,
                 snapshot.clipped ? "LIMIT" : "OK");
    }
}

} // namespace Lab8

void SetupLab8() {
    Lab8::Console.begin(9600);
    initStdio(&Lab8::Console);

    InitializeLed(Lab8::GreenPin);
    InitializeLed(Lab8::RedPin);
    InitializeLed(Lab8::YellowPin);
    InitializeKeypad(Lab8::RowPins, Lab8::ColPins);

    SetLedState(Lab8::GreenPin, true);
    SetLedState(Lab8::RedPin, false);
    SetLedState(Lab8::YellowPin, false);

    Lab8::StateLock = xSemaphoreCreateMutex();
    Lab8::CommandReady = xSemaphoreCreateBinary();

    xTaskCreate(Lab8::TaskCollectCommand, "L8Read", 256, nullptr, 3, nullptr);
    xTaskCreate(Lab8::TaskConditionCommand, "L8Filt", 256, nullptr, 2, nullptr);
    xTaskCreate(Lab8::TaskDriveServo, "L8Servo", 192, nullptr, 2, nullptr);
    xTaskCreate(Lab8::TaskPrintReport, "L8Report", 256, nullptr, 1, nullptr);

    printf_P(PSTR("Lab 8: analog actuator control\n"));
    printf_P(PSTR("Enter 0..100 and press %c, %c clears, %c stops\n"),
             Lab8::CommitKey,
             Lab8::ClearKey,
             Lab8::StopKey);

    vTaskStartScheduler();
}

void LoopLab8() {
}
