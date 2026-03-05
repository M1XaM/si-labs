#include "drivers/SerialStream.h"

void SerialStream::begin(unsigned long baud) {
    Serial.begin(baud);
}

void SerialStream::write(char c) {
    Serial.write(c);
}

char SerialStream::read() {
    return Serial.read();
}

bool SerialStream::available() {
    return Serial.available();
}
