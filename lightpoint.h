#ifndef LIGHTPOINT_H
#define LIGHTPOINT_H

#include "relay.h"

class Lightpoint: public Relay
{
    public:
        Lightpoint(int relay,int button);
        virtual void press(int button,int previousState);
        virtual int respondsToButton(int button);
    private:
        int _button;
};
#endif
