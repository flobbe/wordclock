
#include "LedMatrix.h"

#include "assertions.h"
#include "logging.h"


LedMatrix::LedMatrix()
{
    this->enabled_                = true;
    this->needs_update_           = true;
    this->hour_                   = 0;
    this->minute_                 = 0;
    this->second_                 = 0;
    this->current_state_          = S_SPLASH_SCREEN;
    this->current_splash_idx_     = 0;
    this->current_transition_idx_ = 0;
}

void LedMatrix::setup()
{
    FastLED.addLeds<MATRIX_LED_CHIPSET, MATRIX_LED_PIN, MATRIX_LED_COLOR_ORDER>(this->leds_, LED_CNT);
    FastLED.setBrightness(MATRIX_LED_BRIGHTNESS);
    FastLED.setMaxPowerInMilliWatts(MATRIX_LED_MAX_POWER_MW);
    FastLED.clear();
    FastLED.show();

    uint8_t idx = 0;
    for (uint8_t x = MATRIX_WIDTH/2; x < MATRIX_WIDTH; x++, idx++)
        this->leds_outline_[idx] = & this->leds_[xy(x, 0)];
    for (uint8_t y = 1; y < MATRIX_HEIGHT; y++, idx++)
        this->leds_outline_[idx] = & this->leds_[xy(MATRIX_WIDTH-1, y)];
    for (uint8_t x = MATRIX_WIDTH-2; x > 0; x--, idx++)
        this->leds_outline_[idx] = & this->leds_[xy(x, MATRIX_HEIGHT-1)];
    for (uint8_t y = MATRIX_HEIGHT-1; y > 0; y--, idx++)
        this->leds_outline_[idx] = & this->leds_[xy(0, y)];
    for (uint8_t x = 0; x < MATRIX_WIDTH/2; x++, idx++)
        this->leds_outline_[idx] = & this->leds_[xy(x, 0)];

//    uint8_t splash_count = sizeof(SPLASH_FUNCTIONS) / sizeof(SPLASH_FUNCTIONS[0]);
//    uint8_t trans_count  = sizeof(TRANSITION_FUNCTIONS) / sizeof(TRANSITION_FUNCTIONS[0]);
//    this->current_splash_idx_     = random8(splash_count);
//    this->current_transition_idx_ = random8(trans_count);
//    LOG_DEBUG("LedMatrix: splash function     = %d (of %d)\n", this->current_splash_idx_, splash_count);
//    LOG_DEBUG("LedMatrix: transition function = %d (of %d)\n", this->current_transition_idx_, trans_count);
}

void LedMatrix::update()
{
    if (this->enabled_)
    {

        if (this->needs_update_)
        {

            if (this->current_state_ == S_SPLASH_SCREEN)
            {
                if ((this->*SPLASH_FUNCTIONS[this->current_splash_idx_])())
                    changeState(S_TIME_MODE);
            }
            else if (this->current_state_ == S_TIME_MODE)
            {
                if ((this->*TRANSITION_FUNCTIONS[this->current_transition_idx_])())
                    this->needs_update_ = false;
            }

        }
        else
        {
            EVERY_N_SECONDS(2) nextEffect();
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
        {
            *(this->leds_outline_[this->second_]) = CRGB::Red;
            FastLED.show();
        }
        this->hour_         = h;
        this->minute_       = m;
        this->second_       = s;
        this->needs_update_ = true;
    }
}

void LedMatrix::enabled(const bool e)
{
    if (e != this->enabled_)
    {
        this->enabled_ = e;
        if (e)
            this->needs_update_ = true;
        else
            disableLEDs();
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
    FastLED.clear();
    FastLED.show();
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
    bool finished = false;
    EVERY_N_MILLIS(40)
    {
        uint8_t x          = random8() % MATRIX_WIDTH;
        uint8_t y          = random8() % MATRIX_HEIGHT;
        uint8_t index      = xy(x, y);
        bool    all_active = false;

        uint8_t i = index;
        while (!all_active && this->leds_[i])
        {
            i          = (i + 1) % LED_CNT;
            all_active = (i == index);
        }

        this->leds_[i] = CRGB::White;
        FastLED.show();

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
    bool finished = false;
    EVERY_N_MILLIS(30)
    {
        static uint8_t hue = 0;
        static uint8_t x = 0;
        static uint8_t y = 0;

        // fade everything out a bit
        for (uint8_t i = 0; i < LED_CNT; i++)
            if ((this->leds_[i].r + this->leds_[i].g + this->leds_[i].b) > 180)
                this->leds_[i].fadeToBlackBy(20);

        // and reactivate current pixel
        this->leds_[xy(x, y)] = CHSV(hue, 255, 255);

        FastLED.show();
        hue += 2;

        // move it
        snakeStep(&x, &y);

        finished = (x == 0 && y == 0);
    }
    return finished;
}

bool LedMatrix::splashSnake2()
{
    bool finished = false;
    EVERY_N_MILLIS(30)
    {
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
            this->leds_[xy(x, y)] = CHSV(hue, 255, 255);
            hue -= 2;
            snakeStep(&x, &y);
        }
        FastLED.show();

        finished = (goal_x == 0 && goal_y == 0);
    }
    return finished;
}

bool LedMatrix::transFade()
{
    bool finished = false;
    EVERY_N_MILLIS(30)
    {
        finished = true;
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
            for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
            {
                CRGB &p = this->leds_[xy(x, y)];
                if (this->word_frame_.isSet(x, y))
                {
                    p.addToRGB(5);
                    finished &= (p.r & p.g & p.b) == 0xFF;
                }
                else
                {
                    p.subtractFromRGB(10);
                    finished &= (p.r | p.g | p.b) == 0x00;
                }
            }

        FastLED.show();
    }
    return finished;
}

bool LedMatrix::transSetHard()
{
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
            this->leds_[xy(x, y)] = (this->word_frame_.isSet(x, y)) ? CRGB::White : CRGB::Black;
    FastLED.show();
    return true;
}
