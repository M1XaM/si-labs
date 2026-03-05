#pragma once
#ifdef USE_FREERTOS
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// Shared synchronisation handles — created in MainLab3Rtos before tasks start.
extern SemaphoreHandle_t xPressSemaphore;  // binary: Task1 gives, Task2 takes
extern SemaphoreHandle_t xStatsMutex;      // mutex: protects PressData stats
#endif
