
#include "WordFrame.h"

#include "configuration.h"
#include "assertions.h"


WordFrame::WordFrame()
{
    ASSERT(MATRIX_WIDTH < 16);  // lines are currently represented by uint16_t
    clear();
}

void WordFrame::clear()
{
    for (uint8_t i = 0; i < MATRIX_HEIGHT; i++)
    {
        this->mask_[i] = 0x0000;
    }
}

bool WordFrame::isSet(uint8_t x, uint8_t y)
{
    ASSERT(x < MATRIX_WIDTH);
    ASSERT(y < MATRIX_HEIGHT);
    return ((this->mask_[y]) >> x) & 0x0001;
}

WordFrame& WordFrame::add(Word word)
{
    uint16_t bits = (0x0001 << word.length) - 1;
    this->mask_[word.y] |= bits << word.x;
    return *this;  // to cascade function calls: myFrame.add(x).add(y).add(z);
}

WordFrame& WordFrame::fromTime(uint8_t hour, uint8_t minute)
{
    ASSERT(hour   < 12);
    ASSERT(minute < 60);

    clear();

    uint8_t minute2 = minute;
    if ((minute >= 21) && (minute <= 29))
    {
        minute2 = 30 - minute;
    }
    else if ((minute >= 31) && (minute <= 39))
    {
        minute2 = minute - 30;
    }
    else if ((minute >= 40) && (minute <= 59))
    {
        minute2 = 60 - minute;
    }

    uint8_t hour2 = hour;
    if (minute >= 21)
    {
        hour2 = (hour + 1) % 12;
    }

    add(W0_ES);
    add(W0_IST);

    switch (minute2)
    {
        case  1: add(W0_EINE);    break;
        case  2: add(W1_ZWEI);    break;
        case  3: add(W0_DREI);    break;
        case  4: add(W4_VIER);    break;
        case  5: add(W3_FUENF);   break;
        case  6: add(W3_SECHS);   break;
        case  7: add(W2_SIEBEN);  break;
        case  8: add(W2_ACHT);    break;
        case  9: add(W2_NEUN);    break;
        case 10: add(W4_ZEHN);    break;
        case 11: add(W4_ELF);     break;
        case 12: add(W3_ZWOELF);  break;
        case 13: add(W0_DREI);
                 add(W4_ZEHN);    break;
        case 14: add(W4_VIER);
                 add(W4_ZEHN);    break;
        case 15: add(W4_VIERTEL); break;
        case 16: add(W3_SECH);
                 add(W4_ZEHN);    break;
        case 17: add(W2_SIEB);
                 add(W4_ZEHN);    break;
        case 18: add(W2_ACHT);
                 add(W4_ZEHN);    break;
        case 19: add(W2_NEUN);
                 add(W4_ZEHN);    break;
        case 20: add(W1_ZWANZIG); break;
    }
    if (minute2 == 1)
    {
        add(W5_MINUTE);
    }
    else if (minute % 5 > 0)
    {
        add(W5_MINUTEN);
    }

    if (((minute >= 1) && (minute <= 20)) || ((minute >= 31) && (minute <= 39)))
    {
        add(W6_NACH);
    }
    if (((minute >= 21) && (minute <= 29)) || ((minute >= 40) && (minute <= 59)))
    {
        add(W5_VOR);
    }

    if ((minute >= 21) && (minute <= 39))
    {
        add(W6_HALB);
    }

    switch (hour2)
    {
        case  0: add(W10_ZWOELF);  break;
        case  1: add((minute == 0) ? W7_EIN : W7_EINS); break;
        case  2: add(W8_ZWEI);     break;
        case  3: add(W8_DREI);     break;
        case  4: add(W10_VIER);    break;
        case  5: add(W8_FUENF);    break;
        case  6: add(W7_SECHS);    break;
        case  7: add(W7_SIEBEN);   break;
        case  8: add(W9_ACHT);     break;
        case  9: add(W9_NEUN);     break;
        case 10: add(W9_ZEHN);     break;
        case 11: add(W6_ELF);      break;
    }

    if (minute == 0)
    {
        add(W10_UHR);
    }

    return *this;  // to cascade function calls: myFrame.fromTime(x, y).add(z);
}
