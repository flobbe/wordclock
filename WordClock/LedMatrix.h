#ifndef __LEDMATRIX_H
#define __LEDMATRIX_H

#include <Arduino.h>
#include <FastLED.h>

#include "configuration.h"
#include "WordFrame.h"

#define LED_CNT (MATRIX_WIDTH * MATRIX_HEIGHT)

class LedMatrix
{

public:

    LedMatrix();

    void setup();

    void update();

    void setTime(const uint8_t hour, const uint8_t minute, const uint8_t second);

    void enabled(const bool state);

private:

    typedef enum {
        S_SPLASH_SCREEN = 0,
        S_TIME_MODE     = 1
    } State;

    typedef bool (LedMatrix::*EffectFunc)(void);

    const EffectFunc SPLASH_FUNCTIONS[2]     = { &LedMatrix::splashRandom,
                                                 &LedMatrix::splashSnake2 };
    const EffectFunc TRANSITION_FUNCTIONS[1] = { &LedMatrix::transFade };

    bool      enabled_;
    bool      needs_update_;
    uint8_t   hour_;
    uint8_t   minute_;
    uint8_t   second_;
    WordFrame word_frame_;
    CRGB      leds_[LED_CNT];
    CRGB*     leds_outline_[(MATRIX_WIDTH + MATRIX_HEIGHT - 2) * 2];
    State     current_state_;
    uint8_t   current_splash_idx_;
    uint8_t   current_transition_idx_;

    void changeState(const State new_state);

    void nextEffect();

    void disableLEDs();

    uint16_t xy(const uint8_t x, const uint8_t y);

    const void snakeStep(uint8_t *x, uint8_t *y);

    // splash screen functions:
    bool splashRandom();
    bool splashSnake();
    bool splashSnake2();

    // time transition functions:
    bool transFade();
    bool transSetHard();

};

#endif  // __LEDMATRIX_H
