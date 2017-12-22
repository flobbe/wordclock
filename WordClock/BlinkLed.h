#ifndef __BLINKLED_H
#define __BLINKLED_H

#include <avr/io.h>

#include "configuration.h"

class BlinkLed
{

public:

    BlinkLed(unsigned long high_millis, unsigned long low_millis);

    void setup();

    void setInterval(unsigned long high_millis, unsigned long low_millis);

    void update();

private:

    uint8_t       state_;
    unsigned long previous_millis_;
    unsigned long high_millis_;
    unsigned long low_millis_;

    void setLed(uint8_t state);

};

#endif  // __BLINKLED_H
