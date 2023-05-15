#pragma once
#include <Adafruit_SH110X.h>
#include "WWConf.hpp"

#define MAX_MSG_COUNT 5 // max lines of messages to display

class DisplayManager {
  public:
    DisplayManager();

    ~DisplayManager();

    void refresh(int hour, int minute, bool isGPSFix, int scannedAPs, int nUnprotected, int nWEP);

    void updateClock(int hour, int minute, bool update=true);
    void updateGPS(bool isGPSFix, bool update=true);
    void updateStats(int scannedAPs, int nUnprotected, int nWEP, bool update=true);
    void updateMsgs(bool update=true);

    void pushMsg(String msg);
    void overwriteMsg(String msg);
    
  private:
    Adafruit_SH1106G* m_display;
    String m_messages[MAX_MSG_COUNT];
    int m_msgCount;
};
