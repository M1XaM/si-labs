#include "drivers/LcdStream.h"
#include "drivers/LcdDriver.h"
#include "drivers/KeypadDriver.h"

/* ---- IStream implementation for LCD + keypad --------------------------- */

void LcdStream::write(char ch)
{
    LcdPrintChar(ch);
}

char LcdStream::read()
{
    return ScanKeypad();
}

bool LcdStream::available()
{
    return IsKeypadKeyAvailable();
}
