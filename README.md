# Flo's word clock

This is my DIY version of a word clock, which shows the time as it is spoken.
It consists of 143 LEDs (13 x 11) behind a predefined letter layout.
Unlike many other word clock implementations it can express every minute of the day (not just every fifth).


## Parts list

I used the following components:

 - 143 WS2812B LEDs on a strip with 60 LEDs/meter, so you need 3 meters
 - 5 V Power supply with about 8 A output current (I used a [MeanWell LPV-60-5](https://www.meanwell-web.com/en-gb/ac-dc-single-output-led-driver-constant-voltage-cv-lpv--60--5))
   - each of the 143 LEDs can draw about 50-60 mA (but normally the total consumption is below 500mA)
 - [Teensy 3.1](https://www.pjrc.com/teensy/teensy31.html) 32 bit microcontroller board
 - LM2596 DC-DC Step-Down converter
 - DCF77 time signal receiver module
 - RTC DS3231 Real-Time-Clock module
 - Picture frame that can hold 30 x 30 x 1.5 cm
 - 3D printed socket for the LEDs, see below
 - frosted acrylic glass (3 mm) to diffuse the light (alternatively semi-transparent paper will also work)
 - The letter matrix made of black cardboard cut out by laser, see below
