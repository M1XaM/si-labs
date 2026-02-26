#include "drivers/KeypadDriver.h"

/* ---- internal state ---------------------------------------------------- */
static uint8_t  _rPins[KPAD_ROWS];
static uint8_t  _cPins[KPAD_COLS];
static char     _bufferedChar = '\0';

/* 4x4 look-up table that maps (row, col) to the printed character */
static const char _charTable[KPAD_ROWS][KPAD_COLS] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' },
};

/* ---- public API -------------------------------------------------------- */

void InitializeKeypad(const uint8_t rowPins[KPAD_ROWS],
                      const uint8_t colPins[KPAD_COLS])
{
    uint8_t idx;

    for (idx = 0; idx < KPAD_ROWS; ++idx) {
        _rPins[idx] = rowPins[idx];
        pinMode(rowPins[idx], OUTPUT);
        digitalWrite(rowPins[idx], HIGH);
    }

    for (idx = 0; idx < KPAD_COLS; ++idx) {
        _cPins[idx] = colPins[idx];
        pinMode(colPins[idx], INPUT_PULLUP);
    }
}

bool IsKeypadKeyAvailable()
{
    if (_bufferedChar != '\0')
        return true;

    for (uint8_t row = 0; row < KPAD_ROWS; ++row)
    {
        digitalWrite(_rPins[row], LOW);

        for (uint8_t col = 0; col < KPAD_COLS; ++col)
        {
            if (digitalRead(_cPins[col]) != LOW)
                continue;

            delay(KPAD_DEBOUNCE);

            if (digitalRead(_cPins[col]) != LOW)
                continue;

            _bufferedChar = _charTable[row][col];

            /* wait until the user lifts the finger */
            while (digitalRead(_cPins[col]) == LOW) { /* spin */ }

            delay(KPAD_RELEASE);
            digitalWrite(_rPins[row], HIGH);
            return true;
        }

        digitalWrite(_rPins[row], HIGH);
    }

    return false;
}

char ScanKeypad()
{
    char ch       = _bufferedChar;
    _bufferedChar = '\0';
    return ch;
}
