#include "drivers/LcdStream.h"
#include "drivers/LcdDriver.h"
#include "drivers/KeypadDriver.h"

void LcdStream::write(char c) {
    LcdPrintChar(c);
}

char LcdStream::read() {
    return ScanKeypad();
}

bool LcdStream::available() {
    return IsKeypadKeyAvailable();
}
