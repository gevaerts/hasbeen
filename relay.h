#ifndef RELAY_H
#define RELAY_H
class Relay
{
    public:
        Relay(int relay);
        virtual void press(int button,int previousState) {};
        virtual int respondsToButton(int button) {};
    protected:
        void setOn(int state);
        int relayState();
    private:
        int _relay;
        int _relayState;
};
#endif
