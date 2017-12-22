#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H


#define DEBUG                                // set log level to verbose
//#define DEBUG_DCF77_SIGNAL                   // determine DCF77 signal quality and print detailed info

#define DCF77_PIN                 2          // Connection pin to DCF77 device
#define DCF77_INTERRUPT           DCF77_PIN  // Interrupt number associated with pin

#define SYNC_INTERVAL_FROM_DCF77  86400      // update RTC from DCF77 every 7 days
#define SYNC_INTERVAL_FROM_RTC    300        // update system clock from RTC every 5 minutes

#define BLINK_LED_PIN             13         // teensy blink led

#define MATRIX_WIDTH              13
#define MATRIX_HEIGHT             11
#define MATRIX_LED_PIN            11
#define MATRIX_LED_CHIPSET        WS2812B
#define MATRIX_LED_COLOR_ORDER    GRB
#define MATRIX_LED_BRIGHTNESS     10         // max led brightness (0..255)
#define MATRIX_LED_MAX_POWER_MW   4000       // limit led power draw to 4W (800mA @ 5V)


#endif  // __CONFIGURATION_H
