#include "labs/lab10/MainLab10.h"

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Servo.h>
#include <ctype.h>
#include <semphr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <task.h>

#include "drivers/KeypadDriver.h"
#include "drivers/LedDriver.h"
#include "drivers/NtcSensor.h"
#include "drivers/SerialStream.h"
#include "labs/lab10/Lab10Shared.h"
#include "services/StdioRedirect.h"

namespace Lab10 {

PidState State = {
    0, 0, DefaultSetPointC10,
    DefaultKpQ88, DefaultKiQ88, DefaultKdQ88,
    0, 0, 0, ServoCenter
};
SemaphoreHandle_t StateLock = nullptr;

static SerialStream Console;
static Servo OutputServo;

int16_t clampSetPoint(int16_t value) {
    if (value < MinSetPointC10) {
        return MinSetPointC10;
    }
    if (value > MaxSetPointC10) {
        return MaxSetPointC10;
    }
    return value;
}

int16_t calculatePid(int16_t setPoint,
                     int16_t measured,
                     int32_t kpQ88,
                     int32_t kiQ88,
                     int32_t kdQ88,
                     int32_t* integral,
                     int16_t* previousError) {
    const int16_t error = (int16_t)(setPoint - measured);
    int32_t nextIntegral = *integral + error;
    if (nextIntegral > IntegralLimit) {
        nextIntegral = IntegralLimit;
    } else if (nextIntegral < -IntegralLimit) {
        nextIntegral = -IntegralLimit;
    }

    const int16_t derivative = (int16_t)(error - *previousError);
    const int32_t scaledOutput = kpQ88 * (int32_t)error +
                                 kiQ88 * nextIntegral +
                                 kdQ88 * (int32_t)derivative;
    int32_t output = scaledOutput >> 8;
    if (output > OutputLimit) {
        output = OutputLimit;
    } else if (output < -OutputLimit) {
        output = -OutputLimit;
    }

    *integral = nextIntegral;
    *previousError = error;
    return (int16_t)output;
}

bool parseQ88(const char* text, int32_t* result) {
    if (text == nullptr || *text == '\0') {
        return false;
    }

    int sign = 1;
    if (*text == '-') {
        sign = -1;
        ++text;
    } else if (*text == '+') {
        ++text;
    }

    bool hasDigit = false;
    int32_t integer = 0;
    while (*text >= '0' && *text <= '9') {
        integer = integer * 10 + (*text - '0');
        hasDigit = true;
        ++text;
    }

    int32_t numerator = 0;
    int32_t denominator = 1;
    if (*text == '.') {
        ++text;
        for (uint8_t i = 0; i < 4 && *text >= '0' && *text <= '9'; ++i) {
            numerator = numerator * 10 + (*text - '0');
            denominator *= 10;
            hasDigit = true;
            ++text;
        }
    }

    if (!hasDigit || *text != '\0') {
        return false;
    }

    *result = sign * (integer * 256 + (numerator * 256 + denominator / 2) / denominator);
    return true;
}

static void printSetPoint(int16_t value) {
    printf_P(PSTR("[SP=%d.%d C]\n"), value / 10, abs(value) % 10);
}

static void nudgeSetPoint(int16_t deltaC10) {
    xSemaphoreTake(StateLock, portMAX_DELAY);
    State.setPointC10 = clampSetPoint(State.setPointC10 + deltaC10);
    const int16_t sp = State.setPointC10;
    xSemaphoreGive(StateLock);
    printSetPoint(sp);
}

static void setPointFromDegrees(long degrees) {
    xSemaphoreTake(StateLock, portMAX_DELAY);
    State.setPointC10 = clampSetPoint((int16_t)(degrees * 10));
    const int16_t sp = State.setPointC10;
    xSemaphoreGive(StateLock);
    printSetPoint(sp);
}

static void resetPidMemory() {
    xSemaphoreTake(StateLock, portMAX_DELAY);
    State.integral = 0;
    State.previousError = 0;
    State.output = 0;
    xSemaphoreGive(StateLock);
    printf_P(PSTR("[PID reset]\n"));
}

static void setGain(char gainName, int32_t q88) {
    xSemaphoreTake(StateLock, portMAX_DELAY);
    if (gainName == 'P') {
        State.kpQ88 = q88;
    } else if (gainName == 'I') {
        State.kiQ88 = q88;
    } else {
        State.kdQ88 = q88;
    }
    State.integral = 0;
    State.previousError = 0;
    xSemaphoreGive(StateLock);

    const long scaled = (long)q88 * 10000L / 256L;
    printf_P(PSTR("[%c=%ld.%04ld]\n"), gainName, scaled / 10000L, labs(scaled) % 10000L);
}

static void processSerialLine(char* line) {
    while (*line == ' ' || *line == '\t') {
        ++line;
    }
    if (*line == '\0') {
        return;
    }

    const char head = (char)toupper((unsigned char)*line);
    if (head == 'P' || head == 'I' || head == 'D') {
        char* valueText = line + 1;
        while (*valueText == ' ' || *valueText == '\t' || *valueText == '=') {
            ++valueText;
        }

        int32_t parsed = 0;
        if (parseQ88(valueText, &parsed)) {
            setGain(head, parsed);
        } else {
            printf_P(PSTR("[bad gain]\n"));
        }
        return;
    }

    if (strncmp(line, "RESET", 5) == 0 || head == 'R') {
        resetPidMemory();
        return;
    }

    char* end = nullptr;
    long degrees = strtol(line, &end, 10);
    if (end != line && (*end == '\0' || *end == ' ' || *end == '\t')) {
        setPointFromDegrees(degrees);
    }
}

static void flushKeypadNumber(char* buffer, uint8_t* length) {
    if (*length == 0) {
        return;
    }

    buffer[*length] = '\0';
    long degrees = strtol(buffer, nullptr, 10);
    setPointFromDegrees(degrees);
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

static void TaskReadCommands(void*) {
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
                if (serialLength > 0) {
                    serialBuffer[serialLength] = '\0';
                    processSerialLine(serialBuffer);
                    serialLength = 0;
                }
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
                flushKeypadNumber(keypadBuffer, &keypadLength);
            } else if (key == PlusOneKey) {
                nudgeSetPoint(10);
            } else if (key == MinusOneKey) {
                nudgeSetPoint(-10);
            } else if (key == PlusTenKey) {
                nudgeSetPoint(100);
            } else if (key == MinusTenKey) {
                nudgeSetPoint(-100);
            }
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskRunPid(void*) {
    const TickType_t period = pdMS_TO_TICKS(ControlPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        xSemaphoreTake(StateLock, portMAX_DELAY);
        const int16_t pv = State.temperatureC10;
        const int16_t sp = State.setPointC10;
        const int32_t kp = State.kpQ88;
        const int32_t ki = State.kiQ88;
        const int32_t kd = State.kdQ88;
        int32_t integral = State.integral;
        int16_t previousError = State.previousError;
        xSemaphoreGive(StateLock);

        const int16_t output = calculatePid(sp, pv, kp, ki, kd, &integral, &previousError);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        State.integral = integral;
        State.previousError = previousError;
        State.output = output;
        xSemaphoreGive(StateLock);

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskDriveServo(void*) {
    const TickType_t period = pdMS_TO_TICKS(ActuatorPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();
    bool blink = false;

    OutputServo.attach(ServoPin);
    OutputServo.write(ServoCenter);

    for (;;) {
        xSemaphoreTake(StateLock, portMAX_DELAY);
        const int16_t output = State.output;
        const int16_t error = State.setPointC10 - State.temperatureC10;
        xSemaphoreGive(StateLock);

        int16_t angle = ServoCenter + (int16_t)((int32_t)output * ServoSwing / OutputLimit);
        if (angle < 0) {
            angle = 0;
        } else if (angle > 180) {
            angle = 180;
        }

        OutputServo.write((uint8_t)angle);
        SetLedState(GreenPin, error >= -10 && error <= 10);
        SetLedState(RedPin, output == OutputLimit || output == -OutputLimit);
        blink = !blink;
        SetLedState(YellowPin, blink);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        State.angle = (uint8_t)angle;
        xSemaphoreGive(StateLock);

        vTaskDelayUntil(&lastWake, period);
    }
}

static void TaskReport(void*) {
    const TickType_t period = pdMS_TO_TICKS(ReportPeriodMs);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&lastWake, period);

        xSemaphoreTake(StateLock, portMAX_DELAY);
        const PidState snapshot = State;
        xSemaphoreGive(StateLock);

        const long kp = (long)snapshot.kpQ88 * 10000L / 256L;
        const long ki = (long)snapshot.kiQ88 * 10000L / 256L;
        const long kd = (long)snapshot.kdQ88 * 10000L / 256L;

        printf_P(PSTR("SP:%d\tPV:%d\tOUT:%d\tANG:%u\n"),
                 snapshot.setPointC10,
                 snapshot.temperatureC10,
                 snapshot.output,
                 snapshot.angle);
        printf_P(PSTR("# SP=%d.%d C PV=%d.%d C OUT=%d Kp=%ld.%04ld Ki=%ld.%04ld Kd=%ld.%04ld\n"),
                 snapshot.setPointC10 / 10,
                 abs(snapshot.setPointC10) % 10,
                 snapshot.temperatureC10 / 10,
                 abs(snapshot.temperatureC10) % 10,
                 snapshot.output,
                 kp / 10000L,
                 labs(kp) % 10000L,
                 ki / 10000L,
                 labs(ki) % 10000L,
                 kd / 10000L,
                 labs(kd) % 10000L);
    }
}

} // namespace Lab10

void SetupLab10() {
    Lab10::Console.begin(9600);
    initStdio(&Lab10::Console);
    NtcSensor::begin(Lab10::SensorPin);

    InitializeLed(Lab10::GreenPin);
    InitializeLed(Lab10::RedPin);
    InitializeLed(Lab10::YellowPin);
    InitializeKeypad(Lab10::RowPins, Lab10::ColPins);

    SetLedState(Lab10::GreenPin, true);
    SetLedState(Lab10::RedPin, false);
    SetLedState(Lab10::YellowPin, false);

    Lab10::StateLock = xSemaphoreCreateMutex();

    xTaskCreate(Lab10::TaskAcquireTemperature, "L10Acq", 192, nullptr, 3, nullptr);
    xTaskCreate(Lab10::TaskReadCommands, "L10Cmd", 320, nullptr, 3, nullptr);
    xTaskCreate(Lab10::TaskRunPid, "L10PID", 192, nullptr, 2, nullptr);
    xTaskCreate(Lab10::TaskDriveServo, "L10Act", 192, nullptr, 2, nullptr);
    xTaskCreate(Lab10::TaskReport, "L10Rep", 320, nullptr, 1, nullptr);

    printf_P(PSTR("Lab 10: PID control\n"));
    printf_P(PSTR("Setpoint: NN or keypad NN %c. Tune: P1.5 I0.05 D0.5 RESET\n"),
             Lab10::CommitKey);

    vTaskStartScheduler();
}

void LoopLab10() {
}
