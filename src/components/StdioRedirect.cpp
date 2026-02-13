#include "services/StdioRedirect.h"
#include <stdio.h>
#include <Arduino.h>

static IStream* activeStream = nullptr;
static FILE stdio_stream;

// Forward declarations
static int stream_putchar(char c, FILE* f);
static int stream_getchar(FILE* f);

void initStdio(IStream* stream) {
    activeStream = stream;

    fdev_setup_stream(&stdio_stream, stream_putchar, stream_getchar, _FDEV_SETUP_RW);

    stdout = &stdio_stream;
    stdin  = &stdio_stream;
}

static int stream_putchar(char c, FILE* f) {
    if (!activeStream) return 0;

    if (c == '\n') {
        activeStream->write('\r');
    }

    activeStream->write(c);
    return 0;
}

static int stream_getchar(FILE* f) {
    if (!activeStream) return 0;

    while (!activeStream->available()) {
    }

    return activeStream->read();
}
