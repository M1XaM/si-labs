#include "labs/lab4/Lab4Shared.h"
#include "labs/lab4/ReadingTask.h"
#include "labs/lab4/StatisticsTask.h"
#include "labs/lab4/DisplayTask.h"
#include "drivers/ButtonDriver.h"
#include "drivers/LedDriver.h"
#include "drivers/SerialStream.h"
#include "services/StdioRedirect.h"

// Define shared handles
SemaphoreHandle_t pressSemaphore;
SemaphoreHandle_t statsMutex;

// Define shared data
namespace ReadingData {
    volatile uint32_t lastDuration = 0;
}

namespace StatisticsData {
    volatile uint16_t shortPressesNumber = 0;
    volatile unsigned long shortPressesTotalDuration = 0;
    volatile uint16_t longPressesNumber = 0;
    volatile unsigned long longPressesTotalDuration = 0;
}

static SerialStream serialIo;

void SetupLab4() {
    // Init Drivers
    InitializeButton(PIN_BTN);
    InitializeLed(PIN_LED_R);
    InitializeLed(PIN_LED_G);
    InitializeLed(PIN_LED_Y);

    serialIo.begin(9600);
    initStdio(&serialIo);

    // Create Sync Objects (must be before tasks if tasks run immediately, but scheduler not started yet)
    pressSemaphore = xSemaphoreCreateBinary();
    statsMutex = xSemaphoreCreateMutex();

    if (pressSemaphore == NULL || statsMutex == NULL) {
        printf("Error creating semaphores\n");
        while(1);
    }

    // Create Tasks
    // Stack depths: 128 words = 256 bytes per task. Display needs more for printf.
    // Reading and Stats are light.
    xTaskCreate(ReadingTask::run, "Reading", 128, NULL, 3, NULL);
    xTaskCreate(StatisticsTask::run, "Stats", 128, NULL, 2, NULL);
    xTaskCreate(DisplayTask::run, "Display", 256, NULL, 1, NULL);

    printf("Lab 4: FreeRTOS implementation started\n");

    vTaskStartScheduler();
}

void LoopLab4() {
    // Empty, scheduler takes over.
}
