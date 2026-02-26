#ifndef _LCD_STREAM_H_
#define _LCD_STREAM_H_

#include "interfaces/IStream.h"

/*
 * Bridges the IStream interface to the LCD + keypad hardware.
 * write() -> LCD display
 * read()  -> 4x4 matrix keypad
 */
class LcdStream : public IStream
{
public:
    void write(char ch)  override;
    char read()          override;
    bool available()     override;
};

#endif /* _LCD_STREAM_H_ */
