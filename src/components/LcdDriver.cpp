#include "drivers/LcdDriver.h"
#include <LiquidCrystal.h>

static LiquidCrystal* Lcd      = nullptr;
static uint8_t         LcdCols  = 0;
static uint8_t         LcdRows  = 0;
static uint8_t         CursorCol = 0;
static uint8_t         CursorRow = 0;

void InitializeLcd(uint8_t rs, uint8_t en,
                   uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                   uint8_t cols, uint8_t rows) {
    Lcd       = new LiquidCrystal(rs, en, d4, d5, d6, d7);
    LcdCols   = cols;
    LcdRows   = rows;
    CursorCol = 0;
    CursorRow = 0;
    Lcd->begin(cols, rows);
    Lcd->clear();
}

void LcdClear() {
    CursorCol = 0;
    CursorRow = 0;
    Lcd->clear();
}

void LcdSetCursor(uint8_t col, uint8_t row) {
    CursorCol = col;
    CursorRow = row;
    Lcd->setCursor(col, row);
}

void LcdPrintChar(char c) {
    if (c == '\r') return;

    if (c == '\n') {
        CursorRow++;
        CursorCol = 0;
        if (CursorRow >= LcdRows) {
            CursorRow = 0;
            Lcd->clear();
        }
        Lcd->setCursor(CursorCol, CursorRow);
        return;
    }

    Lcd->print(c);
    CursorCol++;

    if (CursorCol >= LcdCols) {
        CursorCol = 0;
        CursorRow++;
        if (CursorRow >= LcdRows) {
            CursorRow = 0;
            Lcd->clear();
        }
        Lcd->setCursor(CursorCol, CursorRow);
    }
}
