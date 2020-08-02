# Building an AP-Scanner for Warwalking

This project utilizes the ESP8266 on a Node MCU to detect wireless access points. A Neo 6M GPS modul keeps track of the location while a small OLED display allows for realtime status updates on the scanner. Gathered details are saved to a connected micro SD card for later evaluation. 

## Setup

Below is a connection diagram of all the parts for the AP-Scanner.

![Connection Diagram](Connection.png)

The parts are connected using the following pins:
* Display **I2C** (3V3)           
    * SDA <--> D2                  
    * SCL <--> D1                  
* GPS **Serial** (3V3)           
    * RX <--> D3                    
    * TX <--> D4  
* SD Card **SPI** (5V)            
    * SCLK <--> D5                  
    * MISO <--> D6                  
    * MOSI <--> D7                  
    * CS <--> D8
    * *The Node MCU does not feature a 5V pin but when using a 5V supply (i.e. USB default) pin VIN can be used.*

## Getting Started

The Node MCU can be programmed using the Arduino IDE. To choose the correct board add
> `http://arduino.esp8266.com/stable/package_esp8266com_index.json` 

to Files > Preferences > Additional board manager URLs. Once done install the ESP8266 board from Tools > Board > Board Manager. Supsequently select the NodeMCU 1.0 board with the default settings from the new list of boards.
> COM-port issues can be the result of missing drivers. Check the Node MCU instructions in that case.

In order to parse the GPS messages and interface the display install the additional [TinyGPS++](http://arduiniana.org/libraries/tinygpsplus/) and the [ESP8266 Oled driver](https://github.com/ThingPulse/esp8266-oled-ssd1306) library. 

When the sketch is uploaded to the ESP8266 the scanner will immediately start to scan for networks once the GPS module found a signal. The scanner will update every 5 seconds as shown in the following visualization.

![Arduino Flowchart](Flowchart.png)

