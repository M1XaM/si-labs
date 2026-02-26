#ifndef _KEYPAD_DRV_H_
#define _KEYPAD_DRV_H_

#include <Arduino.h>

/* Matrix dimensions */
static const uint8_t KPAD_ROWS       = 4;
static const uint8_t KPAD_COLS       = 4;

/* Timing thresholds (milliseconds) */
static const uint16_t KPAD_DEBOUNCE  = 50;
static const uint16_t KPAD_RELEASE   = 20;

/* Set up row / column GPIO for the 4x4 matrix */
void InitializeKeypad(const uint8_t rowPins[KPAD_ROWS],
                      const uint8_t colPins[KPAD_COLS]);

/* Non-blocking check: is a key press buffered? */
bool IsKeypadKeyAvailable();

/* Return the buffered key (or '\0') */
char ScanKeypad();

#endif /* _KEYPAD_DRV_H_ */
