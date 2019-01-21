#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H


#define WIFI_SSID                 ""   // <-- insert your wifi name here
#define WIFI_PASSWORD             ""   // <-- insert yout wifi password here

#define THIS_HOST_NAME            "wordclock"

#define MQTT_ENABLED              false
#define MQTT_BROKER_IP            "192.168.5.55"
#define MQTT_USERNAME             "my-mqtt-user"
#define MQTT_PASSWORD             "my-mqtt-password"
#define MQTT_DEVICE_ID            THIS_HOST_NAME

#define TIME_NTP_SERVER           "ptbtime1.ptb.de", "ptbtime2.ptb.de", "pool.ntp.org"
#define TIME_POSIX_TIMEZONE_STR   "CET-1CEST,M3.5.0,M10.5.0/3"  // germany/berlin ; see https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
#define TIME_SYNC_INTERVAL_SEC    5 *   60       // update system clock over ntp [in seconds]

#define TIME_SIMULATION           false          // just for development
#define TIME_SIMULATION_FACTOR    20

#define MATRIX_WIDTH              13
#define MATRIX_HEIGHT             11
#define MATRIX_LED_PIN            13
#define MATRIX_LED_CHIPSET        WS2812B
#define MATRIX_LED_COLOR_ORDER    GRB
#define MATRIX_LED_BRIGHTNESS     13           // max led brightness (0..255)


#endif  // __CONFIGURATION_H
