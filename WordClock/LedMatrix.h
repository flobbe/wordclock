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

    typedef enum {
        SECONDS_HIDDEN    = 0,  // do not show seconds
        SECONDS_HAND      = 1,  // draw a second hand
        SECONDS_DOT       = 2,  // draw only the tip of the second hand
        SECONDS_DECIMAL   = 3,  // write the numbers over the entire display
        SECONDS_COUNTDOWN = 4   // like SECONDS_DECIMAL but as countdown
    } SecondMode;

    LedMatrix();

    void setup();

    void update();

    void setTime(const uint8_t hour, const uint8_t minute, const uint8_t second);

    void setSecondsMode(uint8_t seconds_mode);
    void setSplashScreen(uint8_t splash_idx);

    void setBrightness(uint8_t value);
    void setWordColor(uint8_t red, uint8_t green, uint8_t blue);

    void showWifiConnect();
    void showWifiOk();
    void showWifiError();
    void setUpdateProgress(unsigned int progress, unsigned int total);

private:

    typedef enum {
        S_SPLASH_SCREEN   = 0,  // is performed once at startup
        S_WIFI_CONNECT    = 1,  // 
        S_WIFI_OK         = 2,  // 
        S_TIME_MODE       = 3,  // normal mode in which the time is displayed
        S_FWUPDATE_SCREEN = 4,  // is displayed while the software is being flashed
        S_WIFI_ERROR      = 5   // 
    } State;

    typedef bool (LedMatrix::*EffectFunc)(void);

    const EffectFunc SPLASH_FUNCTIONS[2]     = { &LedMatrix::splashRandom,
                                                 &LedMatrix::splashSnake2 };
    const EffectFunc TRANSITION_FUNCTIONS[1] = { &LedMatrix::transFade    };

    const RgbColor BLACK  = RgbColor(  0,   0,   0);
    const RgbColor WHITE  = RgbColor(255, 255, 255);
    const RgbColor RED    = RgbColor(255,   0,   0);
    const RgbColor GREEN  = RgbColor(0,   255,   0);
    const RgbColor BLUE   = RgbColor(  0,   0, 255);
    const RgbColor YELLOW = RgbColor(255, 255,   0);

    const uint8_t LETTERS_WIFI_XY[4][2]  = { {1, 1}, {1, 2}, {4, 3}, {1, 4} };  // W I F I
    const uint8_t LETTERS_NO_XY[2][2]    = { {7, 5}, {10, 5} };                 // N O
    const uint8_t WIFI_SPINNER_XY[14][2] = { {6, 6}, {7, 6}, {8, 6}, {9, 6}, {10, 6},
                                             {10, 7}, {10, 8},
                                             {10, 9}, {9, 9}, {8, 9}, {7, 9}, {6, 9},
                                             {6, 8}, {6, 7} };

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
    uint8_t   seconds_mode_;
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

    void drawSecondHand();
    void drawSecondDigits();

};

#endif  // __LEDMATRIX_H
