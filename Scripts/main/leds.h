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
      digitalWrite(_pin, HIGH);
    }
    
    
    void off() {
      _state = false;
      digitalWrite(_pin, LOW);
    }
  
  private:
    uint8_t _pin;
    bool _state = false;
};