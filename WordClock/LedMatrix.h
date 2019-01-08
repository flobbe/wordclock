#ifndef __LEDMATRIX_H
#define __LEDMATRIX_H

#include <Arduino.h>
#include <NeoPixelBrightnessBus.h>  // "NeoPixelBus" by Makuna (v2.4.1)

#include "configuration.h"
#include "WordFrame.h"

const uint16_t LED_CNT = MATRIX_WIDTH * MATRIX_HEIGHT;

typedef NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> NeoPixelBusType;
static NeoPixelBusType LedMatrix_leds_(LED_CNT, MATRIX_LED_PIN);

class LedMatrix
{

public:

    LedMatrix();

    void setup();

    void update();

    void setTime(const uint8_t hour, const uint8_t minute, const uint8_t second);

    void setSplashScreen(uint8_t splash_idx);

    void setBrightness(uint8_t value);
    void setWordColor(uint8_t red, uint8_t green, uint8_t blue);

    void setUpdateProgress(unsigned int progress, unsigned int total);

private:

    typedef enum {
        S_SPLASH_SCREEN   = 0,  // is performed once at startup
        S_TIME_MODE       = 1,  // normal mode in which the time is displayed
        S_FWUPDATE_SCREEN = 2   // is displayed while the software is being flashed
    } State;

    typedef bool (LedMatrix::*EffectFunc)(void);

    const EffectFunc SPLASH_FUNCTIONS[2]     = { &LedMatrix::splashRandom,
                                                 &LedMatrix::splashSnake2 };
    const EffectFunc TRANSITION_FUNCTIONS[1] = { &LedMatrix::transFade    };

    const RgbColor BLACK = RgbColor(  0,   0,   0);
    const RgbColor WHITE = RgbColor(255, 255, 255);
    const RgbColor RED   = RgbColor(255,   0,   0);
    const RgbColor GREEN = RgbColor(0,   255,   0);
    const RgbColor BLUE  = RgbColor(  0,   0, 255);

    bool      needs_update_;
    uint8_t   hour_;
    uint8_t   minute_;
    uint8_t   second_;
    uint16_t  millis_delta_;
    WordFrame word_frame_;
    NeoPixelBusType& leds_;
    State     current_state_;
    uint8_t   current_splash_idx_;
    uint8_t   current_transition_idx_;
    uint8_t   update_screen_progress_;
    RgbColor  color_words_;

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
