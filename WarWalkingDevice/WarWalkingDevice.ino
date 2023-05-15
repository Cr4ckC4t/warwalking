/**
 * @file WarWalkingDevice.ino
 *
 * @brief Running a handheld AP scanner on a NodeMCU for warwalking.
 *
 * @author crackcat
 *
 */

#include <WiFi.h>
#include <SD.h> // SD card library
#include <TinyGPS++.h> // GPS library
#include "DisplayManager.hpp" // Display library
#include "WWConf.hpp"

DisplayManager* disp;

TinyGPSPlus gps;
struct {
  float lat=0;
  float lng=0;
  int hour=0;
  int minute=0;
  bool isFix=false;
  int year=0;
  int month=0;
  int day=0;
} gpsData;

struct {
  unsigned long total=0;
  unsigned int open=0;
  unsigned int wep=0;
} APStats;

struct AccessPoint {
  String bssid;
  String ssid;
  String encryption;
  int rssi;
  int channel;
  float latitude;
  float longitude;
};

void updateGPSData() {
  gpsData.isFix = false;

  while (!gpsData.isFix) {
    disp->updateGPS(gpsData.isFix);
    while (Serial2.available()) {
      if (gps.encode(Serial2.read())) {
        if (gps.location.isValid() && gps.date.isValid()) {
          gpsData.lat = gps.location.lat();
          gpsData.lng = gps.location.lng();
          gpsData.isFix = true;
          disp->updateGPS(gpsData.isFix);

          gpsData.year = gps.date.year();
          gpsData.month = gps.date.month();
          gpsData.day = gps.date.day();
        }
        if (gps.time.isValid()) {
          gpsData.hour = gps.time.hour();
          gpsData.minute = gps.time.minute();
          disp->updateClock(gpsData.hour, gpsData.minute);
        }
      }
    }
  }
}

String getEncryptionTypeString(int encryption) {
  switch (encryption) {
    // https://github.com/espressif/arduino-esp32/blob/bcc1d758fc343887de03affeedfbb33522ff2523/tools/sdk/esp32/include/esp_wifi/include/esp_wifi_types.h
    case WIFI_AUTH_OPEN:              /**< authenticate mode : open */
      return "OPEN";
    case WIFI_AUTH_WEP:               /**< authenticate mode : WEP */
      return "WEP";
    case WIFI_AUTH_WPA_PSK:           /**< authenticate mode : WPA_PSK */
      return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:          /**< authenticate mode : WPA2_PSK */
      return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:      /**< authenticate mode : WPA_WPA2_PSK */
      return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:   /**< authenticate mode : WPA2_ENTERPRISE */
      return "WPA2_ENTERPRISE";
    case 6:                           /**< authenticate mode : WPA3_PSK */
      return "WPA3_PSK";
    case 7:                           /**< authenticate mode : WPA2_WPA3_PSK */
      return "WPA2_WPA3_PSK";
    case 8:                           /**< authenticate mode : WAPI_PSK */
      return "WAPI_PSK";
    case 9:
      return "MAX";
    default:
      return "UNKNOWN";
  }
}

void setup() {

  /* Set up display */
  disp = new DisplayManager();
  disp->pushMsg("Booting...");

 
  /*Set up SD card */
  disp->pushMsg("Checking SD card...");
  while (!SD.begin(SPI_SD_CS)) {
    delay(1000);
  }
  disp->overwriteMsg("+ SD card ready");


  /* Set up GPS */
  disp->pushMsg("Acquiring GPS...");
  Serial2.begin(GPS_BAUDRATE);
  updateGPSData();
  disp->overwriteMsg("+ GPS ready");

  /* Set up AP scanner */
  disp->pushMsg("Init AP scans...");
  WiFi.mode(WIFI_STA); // activate station mode
  WiFi.disconnect(); // disconnect from any AP
  // prevent (re)connecting to any AP
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false); 
  disp->overwriteMsg("+ AP scanner ready");

  disp->pushMsg("Scan: 0"); // Spaceholder for next overwrite
}

unsigned long scan = 0;
void loop() {
  scan++;
  disp->overwriteMsg("Scan: " + String(scan)); // Overwrite sleep counter

  // Acquire GPS
  updateGPSData();

  // Scan APs
  disp->pushMsg("Scanning APs...");
  int netItems = WiFi.scanNetworks(ASYNC_SCAN, DETECT_HIDDEN);
  disp->overwriteMsg("Scanned "+ String(netItems) + " APs");

  // Process APs
  if (netItems>0) {
    String fileName = String(gpsData.year)+"_"+String(gpsData.month)+"_"+String(gpsData.day)+"-"+LOG_FILE_NAME;
    File logFile = SD.open("/"+fileName,FILE_APPEND); // create or append
    bool cancelLog = false;
    
    for (int ap = 0; ap<netItems; ap++) {
      int enc = WiFi.encryptionType(ap);
      
      AccessPoint newAP;
      newAP.bssid = WiFi.BSSIDstr(ap);
      newAP.ssid = WiFi.SSID(ap);
      newAP.encryption = getEncryptionTypeString(enc);
      newAP.latitude = gpsData.lat;
      newAP.longitude = gpsData.lng;
      newAP.rssi = WiFi.RSSI(ap);
      newAP.channel = WiFi.channel(ap);

      switch (enc) {
        case WIFI_AUTH_OPEN:
          APStats.open++;
          break;
        case WIFI_AUTH_WEP:
          APStats.wep++;
        default:
          break;
      }

      if (!logFile && !cancelLog) {
        disp->pushMsg("[!] SD error");
        cancelLog = true; // avoid repeating SD error in for loop
      } else if (logFile) {
        logFile.println(newAP.bssid+";"+newAP.ssid+";"+newAP.encryption+";"+String(newAP.rssi)+";"+String(newAP.channel)+";"+String(newAP.latitude,6)+";"+String(newAP.longitude,6));
        if (ap<3)
          disp->pushMsg("> "+(newAP.ssid.length()?(newAP.ssid.length()>19?(newAP.ssid.substring(0,16)+"..."):newAP.ssid):"(Hidden)"));
      }
    }
    if (logFile)
      logFile.close();
    
    APStats.total += netItems;
    
    disp->updateStats(APStats.total, APStats.open, APStats.wep);
  }

  // Sleep counter
  int sleepSec = SCAN_INTERVAL_S;
  disp->pushMsg("Next scan in: " + String(sleepSec) + "s");
  while (sleepSec--){
    delay(1000);
    disp->overwriteMsg("Next scan in: " + String(sleepSec) + "s");
  }
}
