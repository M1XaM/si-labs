#ifndef _SERIAL_STREAM_H_
#define _SERIAL_STREAM_H_

#include <Arduino.h>
#include "interfaces/IStream.h"

/*
 * Thin wrapper that routes IStream traffic through
 * the hardware UART (Serial).
 */
class SerialStream : public IStream
{
public:
    /* Open the UART at the requested speed */
    void begin(unsigned long baudRate);

    void write(char ch)  override;
    char read()          override;
    bool available()     override;
};

#endif /* _SERIAL_STREAM_H_ */
