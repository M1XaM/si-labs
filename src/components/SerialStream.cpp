#include "drivers/SerialStream.h"

/* ---- IStream implementation for HW UART ------------------------------- */

void SerialStream::begin(unsigned long baudRate)
{
    Serial.begin(baudRate);
}

void SerialStream::write(char ch)
{
    Serial.write(ch);
}

char SerialStream::read()
{
    return (char)Serial.read();
}

bool SerialStream::available()
{
    return (Serial.available() > 0);
}
