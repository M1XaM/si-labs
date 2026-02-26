#include "services/StdioRedirect.h"
#include <Arduino.h>
#include <stdio.h>

/* ---- file-scope state -------------------------------------------------- */
static IStream  *_ioTarget = nullptr;
static FILE      _avrFile;

/* ---- low-level callbacks for avr-libc ---------------------------------- */

static int _charOut(char ch, FILE *fp)
{
    (void)fp;
    if (_ioTarget == nullptr)
        return 0;

    if (ch == '\n')
        _ioTarget->write('\r');

    _ioTarget->write(ch);
    return 0;
}

static int _charIn(FILE *fp)
{
    (void)fp;
    if (_ioTarget == nullptr)
        return 0;

    while (!_ioTarget->available())
    {
        /* busy-wait until data arrives */
    }
    return _ioTarget->read();
}

/* ---- public API -------------------------------------------------------- */

void initStdio(IStream *ioChannel)
{
    _ioTarget = ioChannel;

    fdev_setup_stream(&_avrFile, _charOut, _charIn, _FDEV_SETUP_RW);

    stdout = &_avrFile;
    stdin  = &_avrFile;
}
