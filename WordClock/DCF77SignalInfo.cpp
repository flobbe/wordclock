
#include "DCF77SignalInfo.h"

#include "logging.h"


DCF77SignalInfo::DCF77SignalInfo()
{
    last_signal_lags_[0] = MAXIMUM_DEVIATION;
    last_signal_lags_[1] = MAXIMUM_DEVIATION;
}


int DCF77SignalInfo::nearestValue(int value, int n1, int n2)
{
    unsigned int d1 = abs(value - n1);
    unsigned int d2 = abs(value - n2);
    if (d1 < d2)
    {
        return n1;
    }
    else
    {
        return n2;
    }
}


// Determins the precision of the last received bit from the DCF77 time station.
// The input signal has to be high for either 100ms or 200ms and low til the
// end of the second is reached: 900ms/800ms or 1900ms/1800ms (at second 59).
// The deviation of the expected timing will be saved in 'last_signal_lags_[]'
// and should not be larger then 50ms.
//   @arg signal: signal from DCF receiver module BN641138
void DCF77SignalInfo::processSignal(uint8_t signal)
{
    static uint8_t signal_d         = 0;  // previous input signal
    static long    last_edge_millis = 0;  // time of the last detected edge
    static int8_t  bit_index        = -1;
    static uint8_t bit_value        = 0;

    // edge detection
    if (signal != signal_d)
    {
        long level_length = millis() - last_edge_millis;

        // time measurement only possible when last_edge_millis is initialized
        if (last_edge_millis >  0)
        {

            int8_t deviation;

            // noisy signals produces more edges
            if (level_length < (100 - MAXIMUM_DEVIATION))
            {
                LOG_VERBOSE("DFC77Info: signal was '%d' for only %2d ms            (  0%%) --> invalid signal\n",
                            signal_d, level_length);
                deviation = MAXIMUM_DEVIATION;
            }
            else
            {
                int expected_length;

                if (signal == LOW)  // falling edge
                {
                    expected_length = nearestValue(level_length, 100, 200);
                    bit_value       = (expected_length == 200) ? 1 : 0;
                }
                else  // rising edge
                {
                    // bit_value==0: high signal length was 100 ms -> 900 or 1900 ms remaining
                    // bit_value==1: high signal length was 200 ms -> 800 or 1800 ms remaining
                    int n1          = (bit_value == 1) ? 800 : 900;
                    expected_length = nearestValue(level_length, n1, n1 + 1000);
                }

                deviation = level_length - expected_length;
                LOG_VERBOSE("DFC77Info: signal was '%d' for %4d ms = %4d%+04d ms (%3d%%)",
                            signal_d, level_length, expected_length, deviation,
                            100 - min(abs(2 * deviation), 100));

                if (signal == LOW)  // falling edge
                {
                    LOG_VERBOSE(" --> ");
                    if (bit_index >= 0)
                    {
                        LOG_VERBOSE("%2d:", bit_index);
                        bit_index++;
                    }
                    LOG_VERBOSE("'%d'", bit_value);
                }
                else  // rising edge
                {
                    if (expected_length > 1000)
                    {
                        LOG_VERBOSE(" --> *** begin of minute ***");
                        bit_index = 0;
                    }
                }

                LOG_VERBOSE("\n");
            }

            last_signal_lags_[1] = last_signal_lags_[0];
            last_signal_lags_[0] = abs(deviation);
        }

        signal_d = signal;
        last_edge_millis = millis();
    }
}


int8_t DCF77SignalInfo::getSignalQuality()
{
    uint8_t this_lag = min(last_signal_lags_[0], MAXIMUM_DEVIATION);
    uint8_t prev_lag = min(last_signal_lags_[1], MAXIMUM_DEVIATION);
    return 100 - (this_lag + prev_lag) * 100 / (2 * MAXIMUM_DEVIATION);
}
