#pragma once
class IStream {
public:
    virtual void write(char c) = 0;
    virtual char read() = 0;
    virtual bool available() = 0;

    virtual ~IStream() {}
};