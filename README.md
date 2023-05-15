# WarWalking

Build a handheld WLAN access point (AP) scanner + dashboard for warwalking.

---

This projected consists of two parts:
1. A handheld device for warwalking (storing scanned APs with their location on a micro SD card) - and
2. A python dashboard (using `tkinter`) to store and visualize the scan results.

And yes, you could also just install an app on your phone that does the same things - this is purely just for fun and giggles.

> Keep in mind that the ESP32 supports WLAN & Bluetooth as well as any basic communication interface - so you could extend this project arbitrarily to make the device do more than just WLAN scanning.

1. [The WarWalking Device](#the-warwalking-device)
2. [The WarWalking Map Viewer](#the-warwalking-map-viewer)

## The WarWalking Device

Below you can see a most basic case that fits all the components in a pluggable connection system for easy maintenance. It's far from being slim-and-light but it works:

![ww-device](https://github.com/Cr4ckC4t/warwalking/assets/63863112/3e8f8dee-9997-44c3-b018-83632a5ff998)

Upon plugging in a micro USB cable for power the device will check for an SD card, search for a GPS signal, and finally start scanning WLAN access points in the vicinity.

![scan-demo-0](https://github.com/Cr4ckC4t/warwalking/assets/63863112/0448143d-5428-45a5-aeda-26f51ffbed6d)

The bottom line will show the total amount of scanned APs.

![scan-demo-1](https://github.com/Cr4ckC4t/warwalking/assets/63863112/3a044bb6-7034-4e1b-8e93-9c3615863d60)

The purely passive scanner will store the following information on the SD card in CSV format (using `;` as delimiter):
- `BSSID;SSID;EncryptionType;RSSI;Channel;Latitude;Longitude`

---

The Arduino IDE requires the following libraries to be installed:
- `Adafruit SH110X` - for the OLED display (requires `Adafruit GFX Library`)
- `TinyGPS++` - for the GPS module
- `WiFi.h` and `SD.h` should come preinstalled.

The board manager used was `esp32` with `ESP32 Dev Module` being the selected board.

Also remember to install the correct USB driver (CP2102 in this case) before programming the uC.

### Parts & Wiring

This project utilizes the following parts for the handheld warwalking device:

- **ESP32 NodeMCU** (38 pins) with a CP2102 chip
- AZDelivery **1,3 Zoll OLED Display** I2C (SSH1106)
- **Neo-6M GPS** module (UART)
- **Micro SD TF Card Module** (6 Pin SPI Interface) - driver module with chip level conversion

Top of the notch quality fritzing diagram:

![wiring-diagram](https://github.com/Cr4ckC4t/warwalking/assets/63863112/1f2140f1-a261-42bc-a03d-fd96736f85d6)

Some GPS modules may have an additional PPS (pulse per second) pin which can remain unconnected.

## The WarWalking Map Viewer

Utilizing the most basic `tkinter` functionality and the [`TkinterMapView` library](https://github.com/TomSchimansky/TkinterMapView), this tool visualizes the collected APs. The python (3.10) program uses `sqlite3` to create and manage a local flat-file database to store any APs.

### Usage
On Windows, simply place the log file from the micro SD card in a directory called `NewData` that's in the same directory as the python code and run `python3 WarWalkingMapViewer.py`.
The log file will automatically be ingested and then moved to a new directory called `OldData` for backup purposes. The database will be stored in a new directory called `Database`.

The GUI is pretty straight forward:

![demo-mapviewer](https://github.com/Cr4ckC4t/warwalking/assets/63863112/d14780d4-1e14-487b-83b4-07186863763d)

Note that adding lots of access points (>500) will quickly lead to performance issues. The used library does not yet support grouping or similar options so markers will only be created for the currently visible view.

---

