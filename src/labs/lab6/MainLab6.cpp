#include "labs/lab6/MainLab6.h"
#include "labs/lab6/Lab6Shared.h"
#include "labs/lab6/TaskSensorAcquisition.h"
#include "labs/lab6/TaskAnalogConditioning.h"
#include "labs/lab6/TaskDisplayReport.h"
#include <Arduino.h>
#include "drivers/SerialStream.h"

// External helper for stdio redirection
extern void initStdio(IStream* stream);

static SerialStream serialIo;

void SetupLab6() {
    serialIo.begin(115200);
    initStdio(&serialIo);
    
    printf("\n--- Starting Lab 6: Analog Signal Conditioning ---\n");

    g_sensorDataMutexLab6 = xSemaphoreCreateMutex();
    if (g_sensorDataMutexLab6 == NULL) {
        printf("Error: Could not create mutex for Lab 6\n");
        while(1);
    }

    // Create Tasks
    // Reduced stack sizes to fit into limited RAM (2KB on Arduino Uno)
    
    // Acquisition: Higher priority to ensure timely sampling
    if (xTaskCreate(TaskSensorAcquisitionLab6, "SensorAcqLab6", 128, NULL, 3, NULL) != pdPASS) {
        printf("Error creating SensorAcqLab6\n");
    }
    
    // Conditioning: Medium priority
    if (xTaskCreate(TaskAnalogConditioningLab6, "AnalogCondLab6", 192, NULL, 2, NULL) != pdPASS) {
        printf("Error creating AnalogCondLab6\n");
    }
    
    // Reporting: Lower priority
    if (xTaskCreate(TaskDisplayReportLab6,     "DispReportLab6", 192, NULL, 1, NULL) != pdPASS) {
        printf("Error creating DispReportLab6\n");
    }

    vTaskStartScheduler();
}

void LoopLab6() {
    // Scheduler manages execution
}
