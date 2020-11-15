
#include "LedMatrix.h"

#if defined(ESP32)
    #include <WiFi.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
#endif

#include "assertions.h"


LedMatrix::LedMatrix() :
  leds_(LedMatrix_leds_)
{
    this->needs_update_           = true;
    this->hour_                   = 0;
    this->minute_                 = 0;
    this->second_                 = 0;
    this->current_state_          = S_SPLASH_SCREEN;
    this->current_splash_idx_     = 0;
    this->current_transition_idx_ = 0;
    this->seconds_mode_           = SECONDS_DOT;
    this->update_screen_progress_ = 0;
    this->color_words_            = WHITE;

    // if analog input pin 0 is unconnected, random analog noise will cause the call to randomSeed() to generate
    // different seed numbers each time the sketch runs. randomSeed() will then shuffle the random function.
    randomSeed(analogRead(0));
}

void LedMatrix::setup()
{
    this->leds_.Begin();
    this->setBrightness(MATRIX_LED_BRIGHTNESS);
    this->leds_.ClearTo({0, 0, 0});
    this->leds_.Show();
}

void LedMatrix::update()
{
    static uint32_t lastTrigger = millis();

    if (this->needs_update_)
    {
        if (this->current_state_ == S_SPLASH_SCREEN)
        {
            if ((this->*SPLASH_FUNCTIONS[this->current_splash_idx_])())
            {
                changeState((WiFi.status() != WL_CONNECTED) ? S_WIFI_CONNECT : S_TIME_MODE);
            }
        }
        else if (this->current_state_ == S_TIME_MODE)
        {
            if ((this->*TRANSITION_FUNCTIONS[this->current_transition_idx_])())
            {
                this->needs_update_ = false;
            }
        }
        else if (this->current_state_ == S_WIFI_CONNECT)
        {
            static uint8_t spinner = 0;
            static uint32_t lastSpin = millis();
            this->leds_.ClearTo(BLACK);
            for (uint8_t i = 0; i < 4; i++)
            {
                this->leds_.SetPixelColor(xy(LETTERS_WIFI_XY[i][0], LETTERS_WIFI_XY[i][1]), WHITE);
            }
            this->leds_.SetPixelColor(xy(WIFI_SPINNER_XY[spinner][0], WIFI_SPINNER_XY[spinner][1]), YELLOW);
            if (millis() - lastSpin > 100)
            {
                lastSpin = millis();
                spinner = (spinner + 1) % 14;
                if (WiFi.status() == WL_CONNECTED)
                {
                    changeState(S_WIFI_OK);
                }
            }
            this->leds_.Show();
        }
        else if (this->current_state_ == S_WIFI_OK)
        {
            this->leds_.ClearTo(BLACK);
            for (uint8_t i = 0; i < 4; i++)
            {
                this->leds_.SetPixelColor(xy(LETTERS_WIFI_XY[i][0], LETTERS_WIFI_XY[i][1]), GREEN);
            }
            this->leds_.Show();
            delay(1000);
            this->leds_.ClearTo(BLACK);
            this->leds_.Show();
            changeState(S_TIME_MODE);
        }
        else if (this->current_state_ == S_WIFI_ERROR)
        {
            static bool toggle = true;
            static uint32_t lastBlink = millis();
            this->leds_.ClearTo(BLACK);
            for (uint8_t i = 0; i < 4; i++)
            {
                this->leds_.SetPixelColor(xy(LETTERS_WIFI_XY[i][0], LETTERS_WIFI_XY[i][1]), toggle ? RED: WHITE);
            }
            for (uint8_t i = 0; i < 2; i++)
            {
                this->leds_.SetPixelColor(xy(LETTERS_NO_XY[i][0], LETTERS_NO_XY[i][1]), toggle ? WHITE : RED);
            }
            if (millis() - lastBlink > 1000)
            {
                lastBlink = millis();
                toggle = ! toggle;
            }
            this->leds_.Show();
        }
    }
    else
    {
        if (millis() - lastTrigger > 2000) // execute every n seconds
        {
            lastTrigger = millis();
            nextEffect();
        }
    }
}

void LedMatrix::setTime(const uint8_t hour, const uint8_t minute, const uint8_t second)
{
    uint8_t h = hour   % 12;
    uint8_t m = minute % 60;
    uint8_t s = second % 60;
    if (h != this->hour_ || m != this->minute_ || s != this->second_)
    {
        word_frame_.fromTime(h, m);
        this->hour_         = h;
        this->minute_       = m;
        this->second_       = s;
        this->millis_delta_ = millis() % 1000;
        this->needs_update_ = true;
    }
}

void LedMatrix::setSecondsMode(uint8_t seconds_mode)
{
    if (seconds_mode != this->seconds_mode_)
    {
        this->seconds_mode_ = seconds_mode;
        this->needs_update_ = true;
    }
}

void LedMatrix::setSplashScreen(uint8_t splash_idx)
{
    this->current_splash_idx_ = splash_idx % 2;
    changeState(S_SPLASH_SCREEN);
}

void LedMatrix::setBrightness(uint8_t value)
{
    this->leds_.SetBrightness(value);
    this->needs_update_ = true;
}

void LedMatrix::setWordColor(uint8_t red, uint8_t green, uint8_t blue)
{
// NOT YET IMPLEMENTED
//    this->color_words_  = RgbColor(red, green, blue);
//    this->needs_update_ = true;
}

void LedMatrix::showWifiConnect()
{
    changeState(S_WIFI_CONNECT);
}
void LedMatrix::showWifiOk()
{
    changeState(S_WIFI_OK);
}
void LedMatrix::showWifiError()
{
    changeState(S_WIFI_ERROR);
}

void LedMatrix::setUpdateProgress(unsigned int progress, unsigned int total)
{
    changeState(S_FWUPDATE_SCREEN);
    uint8_t pos = progress * LED_CNT / total;
    if (pos != this->update_screen_progress_)
    {
        this->update_screen_progress_ = pos;
        RgbColor c = RgbColor::LinearBlend(RED, GREEN, (float)progress / total);
        for (uint8_t i = 0; i < LED_CNT; i++)
            this->leds_.SetPixelColor(xy(i % MATRIX_WIDTH, i / MATRIX_WIDTH), (i <= pos) ? c : BLACK);
        this->leds_.Show();
    }
}

// ----- private methods -----


void LedMatrix::changeState(const State new_state)
{
    if (new_state != this->current_state_)
    {
        this->current_state_ = new_state;
        this->needs_update_  = true;
    }
}

void LedMatrix::nextEffect()
{
    uint8_t n = sizeof(TRANSITION_FUNCTIONS) / sizeof(TRANSITION_FUNCTIONS[0]);
    this->current_transition_idx_ = (this->current_transition_idx_ + 1) % n;
}

void LedMatrix::disableLEDs()
{
    this->leds_.ClearTo(BLACK);
    this->leds_.Show();
}

uint16_t LedMatrix::xy(const uint8_t x, const uint8_t y)
{
    uint8_t new_x;
    if (y & 0x01)
        new_x = (MATRIX_WIDTH - 1) - x;  // odd rows run backwards
    else
        new_x = x;
    return (y * MATRIX_WIDTH) + new_x;
}

bool LedMatrix::splashRandom()
{
    static uint32_t lastTrigger = millis();

    bool finished = false;
    if (millis() - lastTrigger > 20) // execute every n milliseconds
    {
        lastTrigger = millis();
        uint8_t x          = random(MATRIX_WIDTH);
        uint8_t y          = random(MATRIX_HEIGHT);
        uint8_t index      = xy(x, y);
        bool    all_active = false;

        uint8_t i = index;
        while (!all_active && this->leds_.GetPixelColor(i).CalculateBrightness() > 0)
        {
            i          = (i + 1) % LED_CNT;
            all_active = (i == index);
        }

        this->leds_.SetPixelColor(i, this->color_words_);
        this->leds_.Show();
        finished = all_active;
    }
    return finished;
}

const void LedMatrix::snakeStep(uint8_t *x, uint8_t *y)
{
    ASSERT(x != NULL);
    ASSERT(y != NULL);

    if ((*x >= (*y-1)) && (MATRIX_WIDTH-*x-1 > *y) && (*y <= MATRIX_HEIGHT/2))
        (*x)++;
    else if ((MATRIX_WIDTH-*x-1 <= *y) && (*x > *y+2) && (*x > MATRIX_WIDTH/2))
        (*y)++;
    else if ((*x <= *y+2) && (MATRIX_HEIGHT-*y <= *x) && (*y > MATRIX_HEIGHT/2))
        (*x)--;
    else if ((MATRIX_HEIGHT-*y > *x) && (*x < *y-1) && (*x <= MATRIX_WIDTH/2))
        (*y)--;
    else
    {
        *x = 0;
        *y = 0;
    }
}

bool LedMatrix::splashSnake()
{
    static uint32_t lastTrigger = millis();

    bool finished = false;
    if (millis() - lastTrigger > 30) // execute every n milliseconds
    {
        lastTrigger = millis();
        static uint8_t hue = 0;
        static uint8_t x = 0;
        static uint8_t y = 0;

        // fade everything out a bit
        for (uint8_t i = 0; i < LED_CNT; i++)
            if (this->leds_.GetPixelColor(i).CalculateBrightness() > 180)
            {
                RgbColor c = this->leds_.GetPixelColor(i);
                c.Darken(20);
                this->leds_.SetPixelColor(i, c);
            }

        // and reactivate current pixel
        this->leds_.SetPixelColor(xy(x, y), HsbColor(hue, 255, 255));

        this->leds_.Show();
        hue += 2;

        // move it
        snakeStep(&x, &y);

        finished = (x == 0 && y == 0);
    }
    return finished;
}

bool LedMatrix::splashSnake2()
{
    static uint32_t lastTrigger = millis();

    bool finished = false;
    if (millis() - lastTrigger > 30) // execute every n milliseconds
    {
        lastTrigger = millis();
        static uint8_t goal_hue = 0;
        static uint8_t goal_x = 0;
        static uint8_t goal_y = 0;

        snakeStep(&goal_x, &goal_y);
        goal_hue += 2;

        uint8_t hue = goal_hue;
        uint8_t x = 0;
        uint8_t y = 0;
        while (x != goal_x || y != goal_y)
        {
            this->leds_.SetPixelColor(xy(x, y), HsbColor(hue, 255, 255));
            hue -= 2;
            snakeStep(&x, &y);
        }
        this->leds_.Show();

        finished = (goal_x == 0 && goal_y == 0);
    }
    return finished;
}

bool LedMatrix::transFade()
{
    static uint32_t lastTrigger = millis();

    bool finished = false;
    if (millis() - lastTrigger > 0) // execute every n milliseconds
    {
        lastTrigger = millis();
        finished = true;
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                RgbColor p = this->leds_.GetPixelColor(xy(x, y));
                if (this->word_frame_.isSet(x, y))
                {
                    p.Lighten(50);
                    this->leds_.SetPixelColor(xy(x, y), p);
                    finished &= p.CalculateBrightness() >= 255;
                }
                else
                {
                    p.Darken(10);
                    this->leds_.SetPixelColor(xy(x, y), p);
                    finished &= p.CalculateBrightness() <= 0;
                }
            }
        }
        if (this->seconds_mode_ == SECONDS_HAND || this->seconds_mode_ == SECONDS_DOT)
        {
            drawSecondHand();
        }
        else if (this->seconds_mode_ == SECONDS_DECIMAL || this->seconds_mode_ == SECONDS_COUNTDOWN)
        {
            drawSecondDigits();
        }
        this->leds_.Show();
    }
    return finished;
}

bool LedMatrix::transSetHard()
{
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
    {
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
        {
            this->leds_.SetPixelColor(xy(x, y), this->word_frame_.isSet(x, y) ? this->color_words_ : BLACK);
        }
    }
    if (this->seconds_mode_ == SECONDS_HAND || this->seconds_mode_ == SECONDS_DOT)
    {
        drawSecondHand();
    }
    else if (this->seconds_mode_ == SECONDS_DECIMAL || this->seconds_mode_ == SECONDS_COUNTDOWN)
    {
        drawSecondDigits();
    }
    this->leds_.Show();
    return true;
}

void LedMatrix::drawSecondHand()
{
    // helper functions
    auto swap = [](int* a, int* b){ int temp = *a; *a = *b; *b = temp; };
    auto draw = [&](int x, int y, double val){
        if ((x >= 0) && (x < MATRIX_WIDTH) && (y >= 0) && (y < MATRIX_HEIGHT) &&
            (this->seconds_mode_ == SECONDS_HAND || x == 0 || x == MATRIX_WIDTH-1 || y == 0 || y == MATRIX_HEIGHT-1))
        {
            uint8_t v1 = 100 * val;
            uint8_t v2 = 250 * val;
            if (this->word_frame_.isSet(x, y))
                this->leds_.SetPixelColor(xy(x, y), RgbColor(255, 255-v2, 255-v2));
            else
                this->leds_.SetPixelColor(xy(x, y), RgbColor(v1, 0, 0));
        }
    };

    const int HAND_LENGTH = 1500;

    uint16_t ms = (millis() - millis_delta_) % 1000;
    double a = (second_ + ms/1000.0) * 2 * 3.1415 / 60;  // angle of the second hand

    int x0 = MATRIX_WIDTH  / 2;
    int y0 = MATRIX_HEIGHT / 2;
    int x1 = x0 + HAND_LENGTH * sin(a);
    int y1 = y0 - HAND_LENGTH * cos(a);

    // --- Xiaolin Wu's line algorithm ---

    int steep = abs(y1 - y0) > abs(x1 - x0);

    // swap the co-ordinates if slope > 1 or we draw backwards 
    if (steep)
    {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1)
    {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    // compute the slope
    int dx = x1 - x0;
    int dy = y1 - y0;
    double gradient = (double)dy / dx;
    if (dx == 0.0)
        gradient = 1.0;

    int xpxl1 = x0;
    int xpxl2 = x1;
    double intery = y0 + gradient; // first y-intersection for the main loop

    // main loop
    if (steep)
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++)
        {
            draw(floor(intery),   x, floor(intery) - intery + 1);
            draw(floor(intery)+1, x, intery - floor(intery));
            intery += gradient;
        }
    else
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++)
        {
            draw(x, floor(intery),   floor(intery) - intery + 1);
            draw(x, floor(intery)+1, intery - floor(intery));
            intery += gradient;
        }
}

void LedMatrix::drawSecondDigits()
{
    auto draw = [&](int x, int y){
        uint8_t blue_v1 = 100;
        uint8_t blue_v2 = 150;
        uint8_t red_v1  = 0;
        uint8_t red_v2  = 0;
        if (this->seconds_mode_ == SECONDS_COUNTDOWN)
        {
            blue_v1 = 0;
            blue_v2 = 0;
            red_v1  = 200;
            red_v2  = 250;
        }
        if (this->word_frame_.isSet(x, y))
            this->leds_.SetPixelColor(xy(x, y), RgbColor(255-blue_v2, 255-blue_v2-red_v2, 255-red_v2));
        else
            this->leds_.SetPixelColor(xy(x, y), RgbColor(red_v1, 0, blue_v1));
    };

    const bool bits[10][45] = { { 0, 1, 1, 1, 0,   // 0
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  0, 1, 1, 1, 0 },
                                { 0, 0, 1, 1, 0,   // 1
                                  0, 1, 0, 1, 0,
                                  1, 0, 0, 1, 0,
                                  0, 0, 0, 1, 0,
                                  0, 0, 0, 1, 0,
                                  0, 0, 0, 1, 0,
                                  0, 0, 0, 1, 0,
                                  0, 0, 0, 1, 0,
                                  0, 0, 0, 1, 0 },
                                { 0, 1, 1, 1, 0,   // 2
                                  1, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  0, 1, 1, 1, 0,
                                  1, 0, 0, 0, 0,
                                  1, 0, 0, 0, 0,
                                  1, 0, 0, 0, 0,
                                  1, 1, 1, 1, 1 },
                                { 1, 1, 1, 1, 0,   // 3
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  0, 1, 1, 1, 0,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  1, 1, 1, 1, 0 },
                                { 1, 0, 0, 0, 1,   // 4
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  0, 1, 1, 1, 1,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1 },
                                { 1, 1, 1, 1, 1,   // 5
                                  1, 0, 0, 0, 0,
                                  1, 0, 0, 0, 0,
                                  1, 0, 0, 0, 0,
                                  0, 1, 1, 1, 0,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  0, 1, 1, 1, 0 },
                                { 0, 1, 1, 1, 0,   // 6
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 0,
                                  1, 0, 0, 0, 0,
                                  1, 1, 1, 1, 0,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  0, 1, 1, 1, 0 },
                                { 1, 1, 1, 1, 1,   // 7
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 1, 0,
                                  0, 0, 0, 1, 0,
                                  0, 0, 1, 0, 0,
                                  0, 0, 1, 0, 0,
                                  0, 1, 0, 0, 0,
                                  0, 1, 0, 0, 0 },
                                { 0, 1, 1, 1, 0,   // 8
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  0, 1, 1, 1, 0,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  0, 1, 1, 1, 0 },
                                { 0, 1, 1, 1, 0,   // 9
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  0, 1, 1, 1, 1,
                                  0, 0, 0, 0, 1,
                                  0, 0, 0, 0, 1,
                                  1, 0, 0, 0, 1,
                                  0, 1, 1, 1, 0 },
                              };
    uint8_t sec = second_;
    if (this->seconds_mode_ == SECONDS_COUNTDOWN)
    {
        sec = 59 - second_;
        if (second_ < 9)  // hold "00" for 10 seconds
            sec = 0;
    }
    uint8_t sec1  = sec % 10;
    uint8_t sec10 = sec / 10;

    uint8_t sec1_offset_x = 3;

    if ((this->seconds_mode_ != SECONDS_COUNTDOWN) || (sec10 > 0))
    {
        for (int i = 0; i < 45; i++)
        {
            uint8_t x = 1 + i % 5;
            uint8_t y = 1 + i / 5;
            if (bits[sec10][i])
                draw(x, y);
        }
        sec1_offset_x = 6;
    }
    for (int i = 0; i < 45; i++)
    {
        uint8_t x = 1 + sec1_offset_x + i % 5;
        uint8_t y = 1 + i / 5;
        if (bits[sec1][i])
            draw(x, y);
    }
}
