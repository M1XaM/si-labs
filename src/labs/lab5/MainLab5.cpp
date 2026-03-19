#include "labs/lab5/MainLab5.h"
#include "labs/lab5/Lab5Shared.h"
#include "labs/lab5/TaskSensorAcquisition.h"
#include "labs/lab5/TaskThresholdAlerting.h"
#include "labs/lab5/TaskDisplayReport.h"
#include <Arduino.h>
#include "drivers/SerialStream.h"
#include "services/StdioRedirect.h"

// The true header likely lacks the declaration but it's initStdio
void initStdio(IStream* stream);

SensorData g_sensorData = {0, 0.0f, false, false, 0};
SemaphoreHandle_t g_sensorDataMutex = NULL;

static SerialStream serialIo;

#define ALERT_LED_PIN 11

void SetupLab5() {
    serialIo.begin(115200);
    initStdio(&serialIo);
    
    pinMode(ALERT_LED_PIN, OUTPUT);
    digitalWrite(ALERT_LED_PIN, LOW);

    g_sensorDataMutex = xSemaphoreCreateMutex();
    if (g_sensorDataMutex == NULL) {
        printf("Error: Could not create mutex\n");
        return;
    }
    
    // Increased stack depths for safety, log() and printf() can eat stacks
    xTaskCreate(TaskSensorAcquisition, "SensorAcq", 192, NULL, 1, NULL);
    xTaskCreate(TaskThresholdAlerting, "ThreshAlert", 128, NULL, 2, NULL);
    xTaskCreate(TaskDisplayReport,     "DispReport", 256, NULL, 1, NULL);

    printf("Lab 5:\n");
    vTaskStartScheduler();
}

void LoopLab5() {
    // Empty, scheduler takes over.
}

