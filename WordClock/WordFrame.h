#ifndef __WORDFRAME_H
#define __WORDFRAME_H

#include <Arduino.h>

#include "configuration.h"

class WordFrame
{

public:

    typedef struct
    {
        uint8_t x;
        uint8_t y;
        uint8_t length;
    } Word;

    const Word W0_ES      = { 0,  0, 2};
    const Word W0_IST     = { 3,  0, 3};
    const Word W0_DREI    = { 7,  0, 4};
    const Word W0_EIN     = { 9,  0, 3};
    const Word W0_EINE    = { 9,  0, 4};

    const Word W1_ZWANZIG = { 0,  1, 7};
    const Word W1_ZWEI    = { 7,  1, 4};
    const Word W1_EIN     = { 9,  1, 3};
    const Word W1_EINS    = { 9,  1, 4};

    const Word W2_SIEB    = { 0,  2, 4};
    const Word W2_SIEBEN  = { 0,  2, 6};
    const Word W2_NEUN    = { 5,  2, 4};
    const Word W2_NACHT   = { 8,  2, 5};
    const Word W2_NACH    = { 8,  2, 4};
    const Word W2_ACHT    = { 9,  2, 4};

    const Word W3_ZWOELF  = { 0,  3, 5};
    const Word W3_FUENF   = { 4,  3, 4};
    const Word W3_SECH    = { 8,  3, 4};
    const Word W3_SECHS   = { 8,  3, 5};

    const Word W4_VIER    = { 0,  4, 4};
    const Word W4_VIERTEL = { 0,  4, 7};
    const Word W4_ELF     = { 5,  4, 3};
    const Word W4_ZEHN    = { 9,  4, 4};

    const Word W5_MINUTE  = { 1,  5, 6};
    const Word W5_MINUTEN = { 1,  5, 7};
    const Word W5_VOR     = { 9,  5, 3};

    const Word W6_NACH    = { 0,  6, 4};
    const Word W6_NACHT   = { 0,  6, 5};
    const Word W6_ACHT    = { 1,  6, 4};
    const Word W6_HALB    = { 5,  6, 4};
    const Word W6_ELF     = {10,  6, 3};

    const Word W7_EIN     = { 0,  7, 3};
    const Word W7_EINS    = { 0,  7, 4};
    const Word W7_SECHS   = { 3,  7, 5};
    const Word W7_SIEBEN  = { 7,  7, 6};

    const Word W8_FUENF   = { 0,  8, 4};
    const Word W8_ZWEI    = { 4,  8, 4};
    const Word W8_DREI    = { 8,  8, 4};

    const Word W9_ZEHN    = { 1,  9, 4};
    const Word W9_NEUN    = { 4,  9, 4};
    const Word W9_NACHT   = { 7,  9, 5};
    const Word W9_ACHT    = { 8,  9, 4};

    const Word W10_VIER   = { 0, 10, 4};
    const Word W10_ZWOELF = { 4, 10, 5};
    const Word W10_UHR    = {10, 10, 3};

    WordFrame();

    void clear();

    bool isSet(uint8_t x, uint8_t y);

    WordFrame& add(Word word);

    WordFrame& fromTime(uint8_t hour, uint8_t minute);


private:

    uint16_t mask_[MATRIX_HEIGHT];

};

#endif  // __WORDFRAME_H
