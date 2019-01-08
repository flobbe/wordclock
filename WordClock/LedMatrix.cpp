
#include "LedMatrix.h"

#include "assertions.h"


LedMatrix::LedMatrix() :
  leds_(LedMatrix_leds_)
{
    this->needs_update_           = true;
    this->hour_                   = 0;
    this->minute_                 = 0;
    this->second_                 = 0;
    this->current_state_          = S_SPLASH_SCREEN;
    this->current_splash_idx_     = 1;
    this->current_transition_idx_ = 0;
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
                changeState(S_TIME_MODE);
            }
        }
        else if (this->current_state_ == S_TIME_MODE)
        {
            if ((this->*TRANSITION_FUNCTIONS[this->current_transition_idx_])())
            {
                this->needs_update_ = false;
            }
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
    this->leds_.ClearTo({0,0,0});
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
    if (millis() - lastTrigger > 10) // execute every n milliseconds
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
    this->leds_.Show();
    return true;
}
