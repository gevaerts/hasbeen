#ifndef LIGHTPOINT_H
#define LIGHTPOINT_H

#include "relay.h"

class Lightpoint: public Relay
{
    public:
        Lightpoint(int id, int relay,int button);
        Lightpoint(int id, unsigned char *initData);
        virtual ~Lightpoint();
        virtual void press(int button,int previousState);
        virtual int respondsToButton(int button);
        virtual void printInfo();
    protected:
        virtual int saveConfig(unsigned char *initData);
    private:
        void init();
        int _button;
};
#endif
