#pragma once

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// Shared Handles
extern SemaphoreHandle_t pressSemaphore;
extern SemaphoreHandle_t statsMutex;

// Shared Data Structures
namespace ReadingData {
    extern volatile uint32_t lastDuration;
}

namespace StatisticsData {
    extern volatile uint16_t shortPressesNumber;
    extern volatile unsigned long shortPressesTotalDuration;
    extern volatile uint16_t longPressesNumber;
    extern volatile unsigned long longPressesTotalDuration;
}

// Global Objects (or pointers if initialized later)
// (We will use the pin numbers directly in Tasks, or define constants)

constexpr uint16_t PRESS_DURATION_THRESHOLD_MS = 500;
constexpr uint8_t SHORT_BLINK_NUMBER = 5;
constexpr uint8_t LONG_BLINK_NUMBER = 10;

// Pins
constexpr uint8_t PIN_BTN     = 6;
constexpr uint8_t PIN_LED_Y   = 10;
constexpr uint8_t PIN_LED_R   = 11;
constexpr uint8_t PIN_LED_G   = 12;
