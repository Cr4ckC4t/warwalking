#include "SSD1306Wire.h" // display driver
#ifndef LOG_CLASS_GUARD
#define LOG_CLASS_GUARD

#define MAX_LOGS 3
#define DISP_FONT ArialMT_Plain_10

#define dADDR 0x3c  // display address
#define dSDA D2     // display i2c SDA pin
#define dSCL D1     // display i2c SCL pin

class DispLog {
  public:
    DispLog()
      : m_msgCount(0){
        for (int i=0; i < MAX_LOGS; i++){
          m_logMessages[i]="";
        }
    }
    
    ~DispLog(){}

    void setup(){
      m_display = new  SSD1306Wire(dADDR, dSDA, dSCL);
      m_display->init();
      m_display->setFont(DISP_FONT);
      m_display->flipScreenVertically();
    }

    void refreshClock(int h=0, int m=0, bool globalRefresh=false){
      if (!globalRefresh){
        m_display->setColor(BLACK);
        m_display->fillRect(0,0,64,11);
        m_display->setColor(WHITE);
      }
      
      String clk = (h<9?('0'+String(h)):String(h)) + ':' + (m<9?('0'+String(m)):String(m)) + " UTC";
      m_display->drawString(0,0,clk);
      
      if (!globalRefresh){
        m_display->display();
      }
    }

    void refreshGPS(bool g, bool globalRefresh=false){
      if (!globalRefresh){
        m_display->setColor(BLACK);
        m_display->fillRect(64,0,64,11);
        m_display->setColor(WHITE);
      }
      
      String gps = (g?"GPS |||":"GPS ---");
      m_display->drawString(128-m_display->getStringWidth(gps),0,gps);
      
      if (!globalRefresh){
        m_display->display();
      }
    }

    void refreshFoundAPs(int f, bool globalRefresh=false){
      if (!globalRefresh){
        m_display->setColor(BLACK);
        m_display->fillRect(0,13,128,7);
        m_display->setColor(WHITE);
      }
      
      String helperNetworks = "Networks: ";
      String shorter = " ";
      if (f > 9999){
        f = f/1000;
        shorter = "k";
      }
      String networks = helperNetworks + f + shorter;
      m_display->drawString(0,13, networks);
      
      if (!globalRefresh){
        m_display->display();
      }
    }

    void refreshLog(bool globalRefresh=false){
      if (!globalRefresh){
        m_display->setColor(BLACK);
        m_display->fillRect(0,23,128,29);
        m_display->setColor(WHITE);
      }
      int logStartHeight = 22;
      int lineHeight = 9;

      String logTitle = "Log: ";
      m_display->drawString(0, logStartHeight, logTitle);
      for (int i=0; i < MAX_LOGS; i++){
        m_display->drawString(m_display->getStringWidth(logTitle), logStartHeight+i*lineHeight, m_logMessages[i]);
      }
      
      if (!globalRefresh){
        m_display->display();
      }
    }

    /* @Description: update whole log screen 
     * @Params: hour, minute, isGpsFix, foundAPs, noEnc, wepEnc 
     */
    void refresh(int h=0, int m=0, bool g=false, int f=0, int nE=0, int wE=0){
      m_display->clear();
      
      // draw top hline
      m_display->drawLine(0,12,128,12);

      bool globalRefresh=true;
      this->refreshClock(h, m, globalRefresh);
      this->refreshGPS(g, globalRefresh);
      this->refreshFoundAPs(f, globalRefresh);
      this->refreshLog(globalRefresh);
      
      // draw bottom hline
      m_display->drawLine(0,52,128,52);
      
      // draw encryption types
      String hNoEnc = "[no_enc:";
      String hWepEnc = "][wep:";
      String hEnd = "]";
      String encryptions = hNoEnc + nE + hWepEnc + wE+ hEnd;
      m_display->drawString(0, 52, encryptions);
      
      m_display->display();
    }

    void pushMessage(String message){
      m_msgCount++;
      if (m_msgCount <= MAX_LOGS){
        m_logMessages[m_msgCount-1]=message;
      }else{
        // move every message one up
        for(int i=0; i < (MAX_LOGS-1); i++){
          m_logMessages[i]=m_logMessages[i+1];
        }
        // append new message at the end
        m_logMessages[MAX_LOGS-1]=message;
      }
      refreshLog();
    }

    String m_logMessages[MAX_LOGS];
    SSD1306Wire* m_display;

  private:
    int m_msgCount;
};

#endif
