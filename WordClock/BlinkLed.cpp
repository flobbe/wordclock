
#include "BlinkLed.h"

#include <avr/io.h>

#include "configuration.h"

BlinkLed::BlinkLed(unsigned long high_millis, unsigned long low_millis)
{
    this->state_           = LOW;
    this->previous_millis_ = 0;
    setInterval(high_millis, low_millis);
}

void BlinkLed::setup()
{
    pinMode(BLINK_LED_PIN, OUTPUT);
    setLed(LOW);
}

void BlinkLed::setInterval(unsigned long high_millis, unsigned long low_millis)
{
    this->high_millis_ = high_millis;
    this->low_millis_  = low_millis;
}

void BlinkLed::update()
{
    if (this->state_ == LOW)
    {
        if (millis() - this->previous_millis_ >= this->low_millis_)
        {
            this->previous_millis_ += this->low_millis_;
            setLed(HIGH);
        }
    }
    else
    {
        if (millis() - this->previous_millis_ >= this->high_millis_)
        {
            this->previous_millis_ += this->high_millis_;
            setLed(LOW);
        }
    }
}


void BlinkLed::setLed(uint8_t state)
{
    this->state_ = state;
    digitalWrite(BLINK_LED_PIN, state);
}
