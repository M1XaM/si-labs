#pragma once

#include "interfaces/IStream.h"
#include <Arduino.h>

class SerialStream : public IStream {
public:
    void begin(unsigned long baud);

    virtual void write(char c) override;
    virtual char read() override;
    virtual bool available() override;
};
