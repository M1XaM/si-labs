#include "drivers/LcdDriver.h"
#include <LiquidCrystal.h>

/* ---- module-level state ------------------------------------------------ */
static LiquidCrystal *_hw      = nullptr;
static uint8_t        _maxCols = 0;
static uint8_t        _maxRows = 0;
static uint8_t        _posX    = 0;   /* current column */
static uint8_t        _posY    = 0;   /* current row    */

/* ---- helpers ----------------------------------------------------------- */

static void _resetIfOverflow()
{
    if (_posY < _maxRows)
        return;
    _posY = 0;
    _hw->clear();
}

/* ---- public API -------------------------------------------------------- */

void InitializeLcd(uint8_t rs, uint8_t en,
                   uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                   uint8_t cols, uint8_t rows)
{
    _hw      = new LiquidCrystal(rs, en, d4, d5, d6, d7);
    _maxCols = cols;
    _maxRows = rows;
    _posX    = 0;
    _posY    = 0;

    _hw->begin(cols, rows);
    _hw->clear();
}

void LcdClear()
{
    _posX = 0;
    _posY = 0;
    _hw->clear();
}

void LcdSetCursor(uint8_t col, uint8_t row)
{
    _posX = col;
    _posY = row;
    _hw->setCursor(col, row);
}

void LcdPrintChar(char c)
{
    /* ignore carriage-return */
    if (c == '\r')
        return;

    /* handle line-feed: advance row, reset column */
    if (c == '\n')
    {
        _posY += 1;
        _posX  = 0;
        _resetIfOverflow();
        _hw->setCursor(_posX, _posY);
        return;
    }

    /* normal printable character */
    _hw->print(c);
    _posX += 1;

    if (_posX >= _maxCols)
    {
        _posX  = 0;
        _posY += 1;
        _resetIfOverflow();
        _hw->setCursor(_posX, _posY);
    }
}
