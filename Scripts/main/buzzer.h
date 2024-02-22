#ifndef BUZZER_H
#define BUZZER_H

#include "Arduino.h"


class Buzzer {
  public:
    Buzzer(uint8_t pin) : _pin(pin){}
    
    void init() {
      pinMode(_pin, OUTPUT);
      off();
    }
    
    
    void on() {
      _state = true;
      _lastOnAt = millis();
      digitalWrite(_pin, HIGH);
    }
    
    void off() {
      _state = false;
      digitalWrite(_pin, LOW);
    }
    
    
    void shortBeep() {
      on();
      delay(SHORT_BEEP_DURATION);
      off();
    }
    
    
    void normalBeep() {
      on();
      delay(NORMAL_BEEP_DURATION);
      off();
    }
    
    
    void longBeep() {
      on();
      delay(LONG_BEEP_DURATION);
      off();
    }
    
    void twoShortBeeps() {
      shortBeep();
      delay(SHORT_BEEP_DURATION);
      shortBeep();
    }
    
    void twoNormalBeeps() {
      normalBeep();
      delay(NORMAL_BEEP_DURATION);
      normalBeep();
    }
    
    
    void threeShortBeeps() {
      shortBeep();
      delay(SHORT_BEEP_DURATION);
      shortBeep();
      delay(SHORT_BEEP_DURATION);
      shortBeep();
    }
    
    
    void invalidBeep() {
      longBeep();
      delay(SHORT_BEEP_DURATION);
      shortBeep();
    }
    
    
    void validBeep() {
      shortBeep();
      delay(SHORT_BEEP_DURATION);
      shortBeep();
    }
    
  /*
    Time should be in milli seconds
  */
  const uint16_t SHORT_BEEP_DURATION = 100;
  const uint16_t LONG_BEEP_DURATION  = 2000;
  const uint16_t NORMAL_BEEP_DURATION  = 1000;
    
  private:
    uint8_t _pin;
    bool _state = false;
    unsigned long _lastOnAt = 0;
    unsigned long _waitDuration = 0;

};

#endif
