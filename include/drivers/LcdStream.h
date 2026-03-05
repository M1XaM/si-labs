#pragma once
#include "interfaces/IStream.h"

class LcdStream : public IStream {
public:
    virtual void write(char c) override;
    virtual char read()        override;
    virtual bool available()   override;
};
