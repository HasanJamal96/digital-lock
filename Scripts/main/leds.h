#include "Arduino.h"


class Led {
  public:
    Led(uint8_t pin) : _pin(pin){}
          
          
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
    
    
    
    void onForDuration(uint32_t ms) {
      _waitDuration = ms;
      on();
    }  
    
    
    // run this function once in main loop
    void watch() {
      if(_state) {
        if(millis() - _lastOnAt >= _waitDuration) {
          off();
        }
      }
    } 
  
  private:
    uint8_t _pin;
    bool _state = false;
    unsigned long _lastOnAt = 0;
    unsigned long _waitDuration = 0;
};