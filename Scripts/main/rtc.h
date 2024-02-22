#ifndef RTC_H
#define RTC_H

#include <RTClib.h> 
#include <TimeLib.h> 


class MyRtc {
  public:
  
  
    bool init() {
      #if (DEBUG == true && DEBUG_RTC == true)
        Serial.println("[RTC] Initializing");
      #endif
      _working = rtc.begin();
      // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      DateTime now = rtc.now();
        
      int h = now.hour();
      int m = now.minute();
      int s = now.second();
  
      int d   = now.day();
      int mon = now.month();
      int y   = now.year();
    
    
    
     setTime(h, m, s, d, mon, y);
      
      #if (DEBUG == true && DEBUG_RTC == true)
        if(_working) 
          Serial.println("[RTC] Initialized");
        else
          Serial.println("[RTC] Initialization failed");
      #endif
      return _working;
    }

    #if (DEBUG == true && DEBUG_RTC == true)
      void printTime() {
        DateTime now = rtc.now();
          if(_working) 
            Serial.printf("[RTC] Current set time is: %d/%d/%d %d-%d-%d\n", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
          else
            Serial.println("[RTC] Not working");
      }
    #endif
    

    void setRtcTime(uint8_t h, uint8_t m, uint8_t s, uint8_t d, uint8_t mon, uint8_t y) {
      _forceSync = true;
      rtc.adjust(DateTime(y, mon, d, h, m, s));
    }

    bool setRtcTime(uint32_t t) {
      if(_working) {
        _forceSync = true;
        rtc.adjust(DateTime(t));
        DateTime now = rtc.now();
        setTime(now.unixtime());
        printTime();
        return true;
      }
      else  {
        return false;
      }
    }
    
    
    // void loop() {
    //   if(_working) {
    //     _syncTime();
    //   }
    // }
  
  private:
    RTC_DS1307 rtc;
    
    uint8_t _retries = 10;
    bool _working = false;
    bool _forceSync = true;
    unsigned long  _lastSync    = 0;
    const uint16_t _SYNC_AFTER = 3600; // time in seconds
    
    
    // void _syncTime() { // this should run once every midnight or at boot or every time rtc adjusted by server
    //   if(millis() - _lastSync >= _SYNC_AFTER || _forceSync) {
    //     _forceSync = false;
    //     DateTime now = rtc.now();
        
    //     uint8_t h = now.hour();
    //     uint8_t m = now.minute();
    //     uint8_t s = now.second();
        
    //     uint8_t d   = now.day();
    //     uint8_t mon = now.month();
    //     uint8_t y   = now.year();
        
        
        
    //     setTime(h, m, s, d, mon, y);
    //     _lastSync = millis();
    //   }
    // }
};

#endif
