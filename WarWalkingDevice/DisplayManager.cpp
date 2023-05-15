#include "DisplayManager.hpp"

DisplayManager::DisplayManager() :
  m_msgCount(0)
{
  for (int i=0; i<MAX_MSG_COUNT; i++)
    this->m_messages[i] = "";
  
  this->m_display = new Adafruit_SH1106G(DISPLAY_WIDTH,DISPLAY_HEIGHT, &Wire, DISPLAY_RESET_PIN);
  this->m_display->begin(DISPLAY_I2C_ADDR);
  
  this->m_display->clearDisplay();
  this->m_display->setTextSize(1);
  this->m_display->setTextColor(SH110X_WHITE);
  this->m_display->display();

  this->refresh(0,0,false,0,0,0);
}

DisplayManager::~DisplayManager() {
  
}

void DisplayManager::refresh(int hour, int minute, bool isGPSFix, int scannedAPs, int nUnprotected, int nWEP) {
  this->m_display->clearDisplay();

  // draw top hline
  this->m_display->drawLine(0,8,127,8, SH110X_WHITE);

  // draw bottom hline
  this->m_display->drawLine(0,54,127,54, SH110X_WHITE);

  this->updateClock(hour, minute, false);
  this->updateGPS(isGPSFix, false);
  this->updateStats(scannedAPs, nUnprotected, nWEP, false);
  this->updateMsgs(false);

  this->m_display->display();
}

void DisplayManager::updateClock(int hour, int minute, bool update) {
  if (update)
    this->m_display->fillRect(0,0,64,7,SH110X_BLACK);

  this->m_display->setCursor(0, 0);
  String clk = (hour<9?('0'+String(hour)):String(hour)) + ':' + (minute<9?('0'+String(minute)):String(minute)) + " UTC";
  this->m_display->print(clk);

  if (update)
    this->m_display->display();
}

void DisplayManager::updateGPS(bool isGPSFix, bool update) {
  if (update)
    this->m_display->fillRect(65,0,127,7,SH110X_BLACK);

  this->m_display->setCursor(80, 0);
  String gps = (isGPSFix?"GPS":"GPS (?)");
  this->m_display->print(gps);
  if (isGPSFix){
    this->m_display->drawCircle(107, 3, 3, SH110X_WHITE);
    this->m_display->drawLine(107, 0, 107, 6, SH110X_WHITE);
    this->m_display->drawLine(104, 3, 110, 3, SH110X_WHITE);
  }
  
  if (update)
    this->m_display->display();
}

void DisplayManager::updateStats(int scannedAPs, int nUnprotected, int nWEP, bool update) {
  if (update)
    this->m_display->fillRect(0,55,127,8,SH110X_BLACK);

  this->m_display->setCursor(0,56);

  String stats = "-:" + String(nUnprotected) + " WEP:" + String(nWEP) + " | " + String(scannedAPs>99999?((int)scannedAPs/1000):scannedAPs) + (scannedAPs>99999?"k":"");
  this->m_display->print(stats);
  
  if (update)
    this->m_display->display();
}

void DisplayManager::updateMsgs(bool update) {
   if (update)
    this->m_display->fillRect(0,9,127,44,SH110X_BLACK);

  this->m_display->setCursor(0,12);

  for (int i=0; i<MAX_MSG_COUNT; i++)
    this->m_display->println(this->m_messages[i]);
  
  if (update)
    this->m_display->display();
}

void DisplayManager::pushMsg(String msg) {
  this->m_msgCount++;

  if (this->m_msgCount <= MAX_MSG_COUNT){
    this->m_messages[this->m_msgCount - 1] = msg;
  } else {
    for (int i=0; i<(MAX_MSG_COUNT-1); i++)
      this->m_messages[i] = this->m_messages[i+1];
    this->m_messages[MAX_MSG_COUNT-1] = msg;
    this->m_msgCount = MAX_MSG_COUNT;
  }
  this->updateMsgs(true);
}

void DisplayManager::overwriteMsg(String msg) {
  if (this->m_msgCount) {
    this->m_messages[this->m_msgCount - 1] = msg; 
    this->updateMsgs(true);
  } else
    this->pushMsg(msg);
}
