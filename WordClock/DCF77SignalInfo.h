#ifndef __DCF77SIGNALINFO_H
#define __DCF77SIGNALINFO_H

#include <Arduino.h>

#define MAXIMUM_DEVIATION 50  // maximal signal lag in milliseconds -> quality will be 0%

class DCF77SignalInfo
{

public:

    DCF77SignalInfo();

    void processSignal(uint8_t signal);
    int8_t getSignalQuality();


private:

    uint8_t last_signal_lags_[2];

    int nearestValue(int value, int n1, int n2);

};

#endif  // __DCF77SIGNALINFO_H
