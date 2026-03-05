#pragma once
#include <Arduino.h>

void InitializeLcd(uint8_t rs, uint8_t en,
                   uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                   uint8_t cols, uint8_t rows);

void LcdPrintChar(char c);
void LcdClear();
void LcdSetCursor(uint8_t col, uint8_t row);
