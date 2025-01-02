#if !defined(MOCKSERVO_H) && defined(NATIVE)
#define MOCKSERVO_H

#include <Arduino.h>

class Servo {
public:
    Servo() = default;
    virtual ~Servo() = default;
    virtual uint8_t attach(int pin) = 0;
    virtual void writeMicroseconds(int value) = 0;
    virtual int readMicroseconds() = 0;
};

#endif
