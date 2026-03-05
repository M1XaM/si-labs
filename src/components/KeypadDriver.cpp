#include "drivers/KeypadDriver.h"

static uint8_t StoredRowPins[KeypadRowCount];
static uint8_t StoredColPins[KeypadColCount];

static const char KeyMap[KeypadRowCount][KeypadColCount] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static char PendingKey = '\0';

void InitializeKeypad(const uint8_t rowPins[KeypadRowCount],
                      const uint8_t colPins[KeypadColCount]) {
    for (uint8_t r = 0; r < KeypadRowCount; r++) {
        StoredRowPins[r] = rowPins[r];
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], HIGH);
    }
    for (uint8_t c = 0; c < KeypadColCount; c++) {
        StoredColPins[c] = colPins[c];
        pinMode(colPins[c], INPUT_PULLUP);
    }
}

bool IsKeypadKeyAvailable() {
    if (PendingKey != '\0') return true;

    for (uint8_t r = 0; r < KeypadRowCount; r++) {
        digitalWrite(StoredRowPins[r], LOW);

        for (uint8_t c = 0; c < KeypadColCount; c++) {
            if (digitalRead(StoredColPins[c]) == LOW) {
                delay(KeypadDebounceMs);

                if (digitalRead(StoredColPins[c]) == LOW) {
                    PendingKey = KeyMap[r][c];

                    while (digitalRead(StoredColPins[c]) == LOW) {
                    }
                    delay(KeypadReleaseMs);
                    digitalWrite(StoredRowPins[r], HIGH);
                    return true;
                }
            }
        }

        digitalWrite(StoredRowPins[r], HIGH);
    }
    return false;
}

char ScanKeypad() {
    char key = PendingKey;
    PendingKey = '\0';
    return key;
}
