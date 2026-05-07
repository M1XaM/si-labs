#include "labs/lab9/MainLab9.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Servo.h>
#include <semphr.h>
#include <stdio.h>
#include <stdlib.h>
#include <task.h>

#include "drivers/KeypadDriver.h"
#include "drivers/LedDriver.h"
#include "drivers/NtcSensor.h"
#include "drivers/SerialStream.h"
#include "labs/lab9/Lab9Shared.h"
#include "services/StdioRedirect.h"

namespace Lab9 {

ControlState State = {0, 0, DefaultSetPointC10, false, NeutralAngle};
SemaphoreHandle_t StateLock = nullptr;

static SerialStream Console;
static Servo RegulatorServo;

int16_t clampSetPoint(int16_t value) {
    if (value < MinSetPointC10) {
        return MinSetPointC10;
    }
    if (value > MaxSetPointC10) {
        return MaxSetPointC10;
    }
    return value;
}

static void printC10(const char* label, int16_t value) {
    printf_P(PSTR("%s=%d.%dC"), label, value / 10, abs(value) % 10);
}

static void setPointByDelta(int16_t deltaC10) {
    xSemaphoreTake(StateLock, portMAX_DELAY);
    State.setPointC10 = clampSetPoint(State.setPointC10 + deltaC10);
    const int16_t updated = State.setPointC10;
    xSemaphoreGive(StateLock);

    printC10("[SP", updated);
    printf_P(PSTR("]\n"));
}

static void setPointFromWholeDegrees(long degrees) {
    xSemaphoreTake(StateLock, portMAX_DELAY);
    State.setPointC10 = clampSetPoint((int16_t)(degrees * 10));
    const int16_t updated = State.setPointC10;
    xSemaphoreGive(StateLock);

    printC10("[SP", updated);
    printf_P(PSTR("]\n"));
}

static void consumeNumber(char* buffer, uint8_t* length) {
    if (*length == 0) {
        return;
    }

    buffer[*length] = '\0';
    char* end = nullptr;
    long value = strtol(buffer, &end, 10);
    if (end != buffer && *end == '\0') {
        setPointFromWholeDegrees(value);
    }
    *length = 0;
}

static void TaskAcquireTemperature(void*) {
    const TickType_t period = pdMS_TO_TICKS(AcquisitionPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        const uint16_t raw = NtcSensor::readRaw(SensorPin);
        const int16_t temp = NtcSensor::toCelsius10(raw);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        State.adc = raw;
        State.temperatureC10 = temp;
        xSemaphoreGive(StateLock);

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskReadSetPoint(void*) {
    const TickType_t period = pdMS_TO_TICKS(CommandPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();
    char serialBuffer[InputBufferSize];
    char keypadBuffer[InputBufferSize];
    uint8_t serialLength = 0;
    uint8_t keypadLength = 0;

    for (;;) {
        while (Serial.available() > 0) {
            const char c = (char)Serial.read();
            if (c == '\r' || c == '\n') {
                consumeNumber(serialBuffer, &serialLength);
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
            } else if (key == CommitKey) {
                putchar('\n');
                consumeNumber(keypadBuffer, &keypadLength);
            } else if (key == PlusOneKey) {
                setPointByDelta(10);
            } else if (key == MinusOneKey) {
                setPointByDelta(-10);
            } else if (key == PlusTenKey) {
                setPointByDelta(100);
            } else if (key == MinusTenKey) {
                setPointByDelta(-100);
            }
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskApplyHysteresis(void*) {
    const TickType_t period = pdMS_TO_TICKS(ControlPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        xSemaphoreTake(StateLock, portMAX_DELAY);
        const int16_t pv = State.temperatureC10;
        const int16_t sp = State.setPointC10;
        bool nextOutput = State.outputOn;
        xSemaphoreGive(StateLock);

        if (pv < sp - HysteresisC10) {
            nextOutput = true;
        } else if (pv > sp + HysteresisC10) {
            nextOutput = false;
        }

        xSemaphoreTake(StateLock, portMAX_DELAY);
        State.outputOn = nextOutput;
        xSemaphoreGive(StateLock);

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskMoveActuator(void*) {
    const TickType_t period = pdMS_TO_TICKS(ActuatorPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();
    bool blink = false;

    RegulatorServo.attach(ServoPin);
    RegulatorServo.write(NeutralAngle);

    for (;;) {
        xSemaphoreTake(StateLock, portMAX_DELAY);
        const bool enabled = State.outputOn;
        xSemaphoreGive(StateLock);

        const uint8_t angle = enabled ? HeatAngle : CoolAngle;
        RegulatorServo.write(angle);
        SetLedState(GreenPin, !enabled);
        SetLedState(RedPin, enabled);
        blink = !blink;
        SetLedState(YellowPin, blink);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        State.angle = angle;
        xSemaphoreGive(StateLock);

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskReportControl(void*) {
    const TickType_t period = pdMS_TO_TICKS(ReportPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, period);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        const ControlState snapshot = State;
        xSemaphoreGive(StateLock);

        printf_P(PSTR("SP:%d\tPV:%d\tOUT:%d\tANG:%u\n"),
                 snapshot.setPointC10,
                 snapshot.temperatureC10,
                 snapshot.outputOn ? 100 : 0,
                 snapshot.angle);
        printf_P(PSTR("# SP=%d.%d C PV=%d.%d C H=%d.%d C OUT=%s ADC=%u\n"),
                 snapshot.setPointC10 / 10,
                 abs(snapshot.setPointC10) % 10,
                 snapshot.temperatureC10 / 10,
                 abs(snapshot.temperatureC10) % 10,
                 HysteresisC10 / 10,
                 HysteresisC10 % 10,
                 snapshot.outputOn ? "ON" : "OFF",
                 snapshot.adc);
    }
}

} // namespace Lab9

void SetupLab9() {
    Lab9::Console.begin(9600);
    initStdio(&Lab9::Console);
    NtcSensor::begin(Lab9::SensorPin);

    InitializeLed(Lab9::GreenPin);
    InitializeLed(Lab9::RedPin);
    InitializeLed(Lab9::YellowPin);
    InitializeKeypad(Lab9::RowPins, Lab9::ColPins);

    SetLedState(Lab9::GreenPin, true);
    SetLedState(Lab9::RedPin, false);
    SetLedState(Lab9::YellowPin, false);

    Lab9::StateLock = xSemaphoreCreateMutex();

    xTaskCreate(Lab9::TaskAcquireTemperature, "L9Acq", 192, nullptr, 3, nullptr);
    xTaskCreate(Lab9::TaskReadSetPoint, "L9Cmd", 256, nullptr, 3, nullptr);
    xTaskCreate(Lab9::TaskApplyHysteresis, "L9Ctrl", 128, nullptr, 2, nullptr);
    xTaskCreate(Lab9::TaskMoveActuator, "L9Act", 192, nullptr, 2, nullptr);
    xTaskCreate(Lab9::TaskReportControl, "L9Rep", 256, nullptr, 1, nullptr);

    printf_P(PSTR("Lab 9: ON-OFF control with hysteresis\n"));
    printf_P(PSTR("Setpoint: serial NN or keypad NN %c; A/B +/-1, C/D +/-10\n"),
             Lab9::CommitKey);

    vTaskStartScheduler();
}

void LoopLab9() {
}
