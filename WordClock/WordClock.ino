// -*- mode: c -*-

#if defined(ESP32)
    #include <WiFi.h>
    #include <ESPmDNS.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266mDNS.h>
    #include <DNSServer.h>
#else
    #error Only tested with ESP32. OTA and telnet debugging require wifi, so the esp8266 may work as well. I have no idea about other devices.
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <time.h>

#include "configuration.h"
#include "LedMatrix.h"



// --------------------------------------------------

uint32_t gProcessTimeMainLoop;
uint32_t gProcessTimeTaskLoop;

time_t    previous_time = 0;

time_t    last_sync_time;  // last time, system time was updated from ntp server

LedMatrix led_matrix;


// --------------------------------------------------

#define LOG_PRINTFLN(fmt, ...)  printfln_P(PSTR(fmt), ##__VA_ARGS__)
#define LOG_SIZE_MAX 256
void printfln_P(const char *fmt, ...)
{
    char buf[LOG_SIZE_MAX];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf_P(buf, LOG_SIZE_MAX, fmt, ap);
    va_end(ap);
    Serial.println(buf);
}

// --------------------------------------------------

void initWiFi()
{
    Serial.printf("Connecting to WiFi %s ", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.printf(".");
    }
    Serial.printf(" done\n");
    Serial.printf("IP address: ");
    Serial.println(WiFi.localIP());
//    LOG_PRINTFLN("Connected to WiFi");
//    LOG_PRINTFLN("IP: %s", WiFi.localIP().toString().c_str());
}

void getNTPTime()
{
    // request time from ntp server
    configTime(0, 0, TIME_NTP_SERVER);
    setenv("TZ", TIME_POSIX_TIMEZONE_STR, 1);
    tzset();

    // check validity of local time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      LOG_PRINTFLN("received ntp time");
      time(&last_sync_time);
    }
    else
    {
      LOG_PRINTFLN("ERROR: Could not determine the time! Make sure the ntp server is accessible (check wifi and dns).");
      // TODO: set led_matrix to error screen
    }
}

void initOTA()
{
    ArduinoOTA.setHostname(THIS_HOST_NAME);
    ArduinoOTA.onStart([]()
                {
                })
              .onEnd([]()
                {
                    ESP.restart();
                })
              .onProgress([](unsigned int progress, unsigned int total)
                {
                    static uint8_t lastPercent = 100;
                    uint8_t percent = progress / (total / 100);
                    led_matrix.setUpdateProgress(progress, total);
                    if (percent != lastPercent)
                    {
                        Serial.printf("Progress: %u / %u = %u %%\r", progress, total, percent);
                        lastPercent = percent;
                    }
                })
              .onError([](ota_error_t error)
                {
                    if (error == OTA_AUTH_ERROR)         LOG_PRINTFLN("OTAError[%u]: Auth Failed",    error);
                    else if (error == OTA_BEGIN_ERROR)   LOG_PRINTFLN("OTAError[%u]: Begin Failed",   error);
                    else if (error == OTA_CONNECT_ERROR) LOG_PRINTFLN("OTAError[%u]: Connect Failed", error);
                    else if (error == OTA_RECEIVE_ERROR) LOG_PRINTFLN("OTAError[%u]: Receive Failed", error);
                    else if (error == OTA_END_ERROR)     LOG_PRINTFLN("OTAError[%u]: End Failed",     error);
                });
    ArduinoOTA.begin();
}


// the setup routine
void setup()
{
    Serial.begin(115200);

    //          Task function and name, Stack size in bytes, Input Parameters, Priority, Task handle.
    xTaskCreate(taskLED, "LED Task", 10000, NULL, 2, NULL);

    initWiFi();

    getNTPTime();

    xTaskCreate(taskOTA, "OTA Task", 10000, NULL, 1, NULL);
}


// the main loop
void loop()
{
    uint32_t timeBeginLoop = millis();  // for main loop watchdog

    time_t now;

#if TIME_SIMULATION
    delay(1000 / TIME_SIMULATION_FACTOR);
    now = previous_time + 1;
#else
    time(&now);  // get system time (seconds since 1970-01-01)

    if (now - last_sync_time > TIME_SYNC_INTERVAL_SEC)
    {
        getNTPTime();
    }
#endif

    if (now != previous_time)  // update only if time (seconds) has changed
    {
        previous_time = now;

        struct tm *timeinfo = localtime(&now);

        // only print on minute changes
        if (timeinfo->tm_sec == 0)
        {
            LOG_PRINTFLN("%4d-%02d-%02d %02d:%02d:%02d   dst=%d   looptime=%lu ms",
                       timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
                       timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_isdst,
                       gProcessTimeTaskLoop);
        }

        led_matrix.setTime(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    }

    // main loop watchdog
    gProcessTimeMainLoop = (millis() - timeBeginLoop);
}


void taskLED(void* parameter)
{
    led_matrix.setup();
    
    while (true)
    {
        uint32_t timeBeginLoop = millis();

        led_matrix.update();
        vTaskDelay(1);

        gProcessTimeTaskLoop = (millis() - timeBeginLoop);
    }
}
 
void taskOTA(void* parameter)
{
    initOTA();
    while (true)
    {
        ArduinoOTA.handle();
        vTaskDelay(100);
    }
}
