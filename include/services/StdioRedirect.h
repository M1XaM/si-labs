#ifndef _STDIO_REDIRECT_H_
#define _STDIO_REDIRECT_H_

#include "interfaces/IStream.h"

/*
 * Bind avr-libc stdout / stdin to the supplied
 * IStream so that printf / getchar work through it.
 */
void initStdio(IStream *ioChannel);

#endif /* _STDIO_REDIRECT_H_ */