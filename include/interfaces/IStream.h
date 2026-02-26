#ifndef _ISTREAM_IFACE_H_
#define _ISTREAM_IFACE_H_

/*
 * Abstract byte-oriented channel.
 * Implementations must provide all three pure-virtual methods.
 */
class IStream
{
public:
    /* Push one character into the channel */
    virtual void write(char ch) = 0;

    /* Pull one character out of the channel */
    virtual char read() = 0;

    /* Return true when at least one byte is pending */
    virtual bool available() = 0;

    virtual ~IStream() {}
};

#endif /* _ISTREAM_IFACE_H_ */