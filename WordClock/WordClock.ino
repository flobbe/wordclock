// -*- mode: c -*-

#include <avr/io.h>
#include <Time.h>       // http://www.arduino.cc/playground/Code/Time
#include <DS1307RTC.h>  // https://github.com/PaulStoffregen/DS1307RTC
#include <DCF77.h>      // https://github.com/thijse/Arduino-Libraries/downloads

#include "configuration.h"
#include "assertions.h"
#include "logging.h"
#include "BlinkLed.h"
#include "LedMatrix.h"

#ifdef DEBUG_DCF77_SIGNAL
    #include "DCF77SignalInfo.h"
#endif


// --------------------------------------------------


time_t    system_time = 0;

bool      rtc_found;             // real time clock module is connected and running

DCF77     dcf = DCF77(DCF77_PIN, DCF77_INTERRUPT);
#ifdef DEBUG_DCF77_SIGNAL
    DCF77SignalInfo dcf_info;
#endif

bool      do_sync;               // RTC and system time should be updated by the DCF77 time signal
time_t    last_sync_from_dcf77;  // last time, RTC and system time was updated

BlinkLed  blink_led = BlinkLed(50, 50);

LedMatrix led_matrix;


// --------------------------------------------------


// the setup routine
void setup()
{
    // start serial port
#if (LOG_LEVEL > LOG_LEVEL_NONE)
    LOG_PRINTER.begin(9600);
//    while (!LOG_PRINTER);
#endif

    // DCF77 module
    pinMode(DCF77_PIN, INPUT);
    dcf.Start();  // attach interrupt on input pin

    // RTC module
    RTC.get();                      // try to communicate with the RTC module
    rtc_found = RTC.chipPresent();  // ... and check if it worked
    if (rtc_found)
    {
        setSyncProvider(RTC.get);   // automatically update system time from real time clock module
        setSyncInterval(SYNC_INTERVAL_FROM_RTC);
        last_sync_from_dcf77 = now();
    }
    else
    {
        LOG_WARN("No real time clock found!\n");
        last_sync_from_dcf77 = 0;   // force update from DCF77 signal
    }

    random16_set_seed(RTC.get());

    blink_led.setup();

    led_matrix.setup();
}


// the main loop
void loop()
{
#ifdef DEBUG_DCF77_SIGNAL
    do_sync = true;
    dcf_info.processSignal(digitalRead(DCF77_PIN));
#endif

    time_t time = now();  // system time (seconds since 1970-01-01)

    if (time != system_time)  // update only if time has changed
    {
        system_time = time;

        led_matrix.setTime(hour(), minute(), second());
//        led_matrix.setTime(now() / 3 / 60 % 12, now() / 3 % 60);  // update every 3 seconds

        LOG_DEBUG("time: %d-%02d-%02d %02d:%02d:%02d%s\n",
                  year(), month(), day(), hour(), minute(), second(),
                  (timeStatus() == timeNotSet)    ? " (not set)" :
                  (timeStatus() == timeNeedsSync) ? " (needs sync from rtc)" : "");
#ifdef DEBUG_DCF77_SIGNAL
        LOG_DEBUG("DCF77 signal quality: %3d%%\r\n", dcf_info.getSignalQuality());
#endif
    }

    if (do_sync)
    {
        time_t dcf77_time = dcf.getTime();
        if (dcf77_time != 0)
        {
            LOG_INFO("DCF77 time received: %d-%02d-%02d %02d:%02d:%02d\n",
                     year(dcf77_time), month(dcf77_time), day(dcf77_time),
                     hour(dcf77_time), minute(dcf77_time), second(dcf77_time));
            // update real time clock
            if (RTC.set(dcf77_time))
            {
                setSyncProvider(RTC.get);  // force system time to be updated as well
                LOG_INFO("Real time clock and system time updated.\n");
            }
            else
            {
                setTime(dcf77_time);  // set system time directly from DCF77 signal
                LOG_WARN("Failed to update real time clock!\n");
            }
            last_sync_from_dcf77 = now();
            do_sync = false;
        }
    }
    else if ((now() - last_sync_from_dcf77) > SYNC_INTERVAL_FROM_DCF77)
    {
        LOG_INFO("Sync local time from DCF77 signal...\n");
        led_matrix.update();
        do_sync = true;
    }

    // disable led matrix while dcf77 time signal is processing
    led_matrix.enabled(! do_sync);

    EVERY_N_MILLIS(5)
    {
        if (do_sync)
            blink_led.setInterval(5, 245);  // blink fast while syncing
        else
            blink_led.setInterval(5, 995);  // slow otherwise
        blink_led.update();

        led_matrix.update();
    }

}
