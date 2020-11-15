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

#if MQTT_ENABLED
    #include <MQTT.h>  // "MQTT" by Joel Gaehwiler (v2.4.1) -- https://github.com/256dpi/arduino-mqtt
#endif



// --------------------------------------------------

uint32_t gProcessTimeMainLoop;
uint32_t gProcessTimeTaskLoop;

time_t    previous_time = 0;

time_t    last_sync_time;  // last time, system time was updated from ntp server

LedMatrix led_matrix;

#if MQTT_ENABLED
    WiFiClient network;
    MQTTClient mqttClient;
    TaskHandle_t taskmqtt;
#endif

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
#if MQTT_ENABLED
    mqttClient.publish("tele/" THIS_HOST_NAME "/LOG", buf);
#endif
}

// --------------------------------------------------

void initWiFi()
{
    Serial.printf("Connecting to WiFi %s ", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint8_t timeout = 20; // seconds
    while (WiFi.status() != WL_CONNECTED && timeout > 0)
    {
        delay(1000);
        Serial.printf(".");
        timeout--;
    }
    if (timeout > 0)
    {
      Serial.println(" done");
      LOG_PRINTFLN("Connected to WiFi");
      LOG_PRINTFLN("IP address: %s", WiFi.localIP().toString().c_str());
    }
    else
    {
      Serial.println(" FAILED!");
      led_matrix.showWifiError();
      while (true) { delay(1000); }
    }
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
#if MQTT_ENABLED
                    vTaskSuspend(taskmqtt);
#endif
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

#if MQTT_ENABLED
    xTaskCreate(taskMQTT, "MQTT Task", 10000, NULL, 1, &taskmqtt);
#endif
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


#if MQTT_ENABLED

void messageReceived(String &topic, String &payload)
{
    auto handleIntRequest = [&](String command, String log_name, int min_value, int max_value, void (*callback)(long)) {
        if (topic == "cmnd/" MQTT_DEVICE_ID "/" + command)
        {
            long payloadInt = payload.toInt();
            if (payloadInt >= min_value && payloadInt <= max_value)
            {
                callback(payloadInt);
                mqttClient.publish("tele/" MQTT_DEVICE_ID "/" + command, String(payloadInt));
            }
            else
                LOG_PRINTFLN("invalid argument to set %s '%s'. possible values: [%d..%d]", log_name, payload, min_value, max_value);
        }
    };

    static uint8_t r = 0, g = 0, b = 0;
    LOG_PRINTFLN("incoming: %s - %s", topic.c_str(), payload.c_str());
    handleIntRequest("restart",     "restart uC",       1,   1, [](long int_arg){ ESP.restart(); });
    handleIntRequest("brightness",  "brightness",       0, 100, [](long int_arg){ led_matrix.setBrightness(int_arg * 254 / 100 + 1); });
    handleIntRequest("color/red",   "word color red",   0, 100, [](long int_arg){ r = int_arg * 255 / 100; led_matrix.setWordColor(r, g, b); });
    handleIntRequest("color/green", "word color green", 0, 100, [](long int_arg){ g = int_arg * 255 / 100; led_matrix.setWordColor(r, g, b); });
    handleIntRequest("color/blue",  "word color blue",  0, 100, [](long int_arg){ b = int_arg * 255 / 100; led_matrix.setWordColor(r, g, b); });
    handleIntRequest("seconds",     "seconds mode",     0,   4, [](long int_arg){ led_matrix.setSecondsMode(int_arg); });
    handleIntRequest("splash",      "splash screen",    0,   1, [](long int_arg){ led_matrix.setSplashScreen(int_arg); });
}
void taskMQTT(void* parameter)
{
    mqttClient.begin(MQTT_BROKER_IP, network);
    mqttClient.onMessage(messageReceived);
    while (true)
    {
        if (!mqttClient.connected())
        {
            Serial.print("connecting to MQTT broker ... ");
            if (mqttClient.connect(THIS_HOST_NAME, MQTT_USERNAME, MQTT_PASSWORD))
            {
                Serial.println("done.");
                mqttClient.subscribe("cmnd/" MQTT_DEVICE_ID "/#"); // subscribe to all command topics for this device
            }
            else
            {
                Serial.println("FAILED!");
            }
        }
        mqttClient.loop();
        vTaskDelay(1);
    } // while (true)
}

#endif
