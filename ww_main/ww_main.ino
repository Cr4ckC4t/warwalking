
#include "ESP8266WiFi.h" // scanning functions
#include <TinyGPS++.h> // GPS parser for the Neo6M module
#include <SoftwareSerial.h> // software serial communication for the Neo6M module
#include <SPI.h> // spi communication
#include <SD.h> // SD card and file functions
#include "logClass.h" // display logger



#define INTERVAL_TIME 5000 // scan interval in milliseconds
// longer intervals might require the esp watchdog to be disabled with 
// ESP.wdtDisable() in the setup() 

#define DETECT_HIDDEN true // detect networks with hidden SSIDs
#define ASYNC_SCAN false // use the standard synchronous scan

#define spiCS D8 // spi chip select pin

#define sRX D4 // serial pin rx
#define sTX D3 // serial pin tx
#define GPSBaud 9600 // serial baudrate for Neo6M module

#define LOG_FILE_NAME "ww_log.txt" // name of the log file on the SD card

#define OLD_BSSIDS 99 // amount of old bssids to save for bssid redundancy check



TinyGPSPlus gps;
SoftwareSerial softserial(sRX, sTX);

DispLog* dispLog=new DispLog();

float latitude;       // GPS latitude
float longitude;      // GPS longitude
int hour;             // GPS time hour
int minute;           // GPS time minute
bool isGpsFix;        // GPS fix

int foundAPs;         // counting detected APs
int execScans;        // counting executed scans
int noEnc;            // counting APs without encryption
int wepEnc;           // counting APs with WEP encryption

String lastBSSIDs[OLD_BSSIDS]; // array for already detected bssids
int lastIndex; // index to use lastBSSIDs as a ring buffer
 
unsigned long frameStart;
unsigned long frameEnd; 
unsigned long frameDiff;



void getGPSandTime(){
  isGpsFix=false;
  // retry to find a signal until location is valid
  while(!isGpsFix){
    dispLog->refreshGPS(isGpsFix);
    while(softserial.available()>0){
     if (gps.encode(softserial.read())){
      if (gps.location.isValid()){
        latitude=gps.location.lat();
        longitude=gps.location.lng();
        isGpsFix=true;
      }else{
        isGpsFix=false;
      }
      // determine time
      if (gps.time.isValid()){
        hour=gps.time.hour();
        minute=gps.time.minute();
        dispLog->refreshClock(hour,minute);
      }
     }
    }
  }
  dispLog->refreshGPS(isGpsFix);
}



void setup(){
  
  isGpsFix=false;
  execScans=0;
  foundAPs=0;
  latitude=0;
  longitude=0;
  hour=0;
  minute=0;
  noEnc=0;
  wepEnc=0;
  lastIndex=0;

  dispLog->setup();

  dispLog->refresh();

  // SD card setup
  boolean sdInit = false;
  while(!sdInit){
    if(!SD.begin(spiCS)){
      dispLog->pushMessage("[SD] init error");
      dispLog->pushMessage("[SD] sleep 5s");
      delay(5000);
    }else{
      dispLog->pushMessage("[SD] init done");
      sdInit=true;
    }
  }
  
  // WIFI setup
  WiFi.mode(WIFI_STA); // activate station mode
  WiFi.disconnect(); // disconnect from any AP
  // make sure that we do not (re)connect to any AP
  WiFi.setAutoConnect(false); // turn off auto connections to APs
  WiFi.setAutoReconnect(false); // turn off auto reconnections

  dispLog->pushMessage("[WiFi] init done");
  
  // GPS setup
  softserial.begin(GPSBaud);

  dispLog->pushMessage("[WW] welcome");
  
  delay(1000); // delay for convenience

  dispLog->refresh(hour, minute, isGpsFix, foundAPs, noEnc, wepEnc);

  delay(500);
}



void loop(){
  frameStart = millis();

  dispLog->pushMessage("[GPS] scan");
  // execute gps scan
  getGPSandTime();

  String helperScanMsg = "[WiFi] scan [";
  String helperBracket = "] ";
  String scanMsg= helperScanMsg + execScans + helperBracket;
  dispLog->pushMessage(scanMsg);

  // execute network scan
  int networkItems = WiFi.scanNetworks(ASYNC_SCAN, DETECT_HIDDEN); 

  dispLog->pushMessage("[WiFi] (" + String(networkItems)+ ")");

  execScans++;

  // check if any networks were found and process them
  if (networkItems > 0){ 
    File logFile = SD.open(LOG_FILE_NAME, FILE_WRITE);
    if (logFile){
      for (int network = 0; network < networkItems; network++){
        
        String bssid = WiFi.BSSIDstr(network);

        // check if this AP was found recently
        boolean isFound = false;
        for (int b=0; b < OLD_BSSIDS; b++){
          if (lastBSSIDs[b] == bssid){
            isFound = true;
          }
        }

        // new bssids will be added and temporarily saved
        if (!isFound){
          foundAPs++;
          lastBSSIDs[lastIndex]=bssid;
          if (++lastIndex == OLD_BSSIDS){
            lastIndex = 0;
          }

          // count encryption types
          switch (WiFi.encryptionType(network)){ 
            case ENC_TYPE_NONE:
              noEnc++;
              break;
            case ENC_TYPE_WEP:
              wepEnc++;
              break;
            default:
              break;
          };
        }
   
        String networkInfo = "";
        networkInfo+=String(longitude,6) + ';';
        networkInfo+=String(latitude,6) + ';';
        networkInfo+=WiFi.SSID(network) + ';';
        networkInfo+=WiFi.encryptionType(network) + ';';
        networkInfo+=WiFi.RSSI(network) + ';';
        networkInfo+=bssid + ';';
        networkInfo+=WiFi.channel(network) + ';';
        networkInfo+=(WiFi.isHidden(network)?1:0);
        logFile.println(networkInfo);
      }
      logFile.close();
    }else{
      dispLog->pushMessage("[SD] card error");
    }
  }
  
  // update display
  dispLog->refresh(hour, minute, isGpsFix, foundAPs, noEnc, wepEnc);

  frameEnd = millis();
  frameDiff = frameEnd - frameStart;
  if (frameDiff > 2000){
    dispLog->pushMessage("[SD] 2s safe");
    // now it's safe to unplug without damaging the sd card
  }
  if (frameDiff < INTERVAL_TIME){
    delay(INTERVAL_TIME-(frameDiff));
  }
}
