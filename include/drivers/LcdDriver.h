#ifndef _LCD_DRV_H_
#define _LCD_DRV_H_

#include <Arduino.h>

/* Initialise a 4-bit-mode HD44780 display */
void InitializeLcd(uint8_t rs, uint8_t en,
                   uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                   uint8_t cols, uint8_t rows);

/* Write a single character (handles wrapping / newline) */
void LcdPrintChar(char c);

/* Erase the whole screen and reset the cursor to (0,0) */
void LcdClear();

/* Move the cursor to a specific column / row */
void LcdSetCursor(uint8_t col, uint8_t row);

#endif /* _LCD_DRV_H_ */
