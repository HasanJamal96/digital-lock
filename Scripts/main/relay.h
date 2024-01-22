#define RELAY_ON  LOW
#define RELAY_OFF HIGH
#define LED_ON  HIGH
#define LED_OFF LOW


class Relay {
  public:
    Relay(uint8_t rPin, uint8_t lPin) : _relayPin(rPin), _ledPin(lPin) {}
    
    void begin() {
      pinMode(_relayPin, OUTPUT);
      pinMode(_ledPin, OUTPUT);
      off();
    }
    
    void on() {
      if(_isExternalTriggerActivate)
        return;
      digitalWrite(_relayPin, RELAY_ON);
      digitalWrite(_ledPin, LED_ON);
      state = true;
    }
    
    void off() {
      if(_isExternalTriggerActivate)
        return;
      digitalWrite(_relayPin, RELAY_OFF);
      digitalWrite(_ledPin, LED_OFF);
      state = false;
      _momentary = false;
    }
    
    
    void momentaryOnFor(uint32_t ms) {
      if(_isExternalTriggerActivate)
        return;
      _momentary = true;
      _onTime = ms * 1000;
      _onAt = millis();
      on();
    }
    
    
    void toggle() {
      if(_isExternalTriggerActivate)
        return;
      _momentary = false;
      if(state) {
        off();
      }
      else {
        on();
      }
    }
    
    bool status() {
      return state;
    }
    
    void loop() {
      if(_isExternalTriggerActivate)
        return;
      if(_momentary) {
        if(millis() - _onAt >= _onTime) {
          off();
        }
      }
    }

    void activateByExternalInput() {
      on();
      _isMomentaryPreviously = _momentary;
      _previousState = state;
      _momentary = false;
      _isExternalTriggerActivate = true;
    }

    void deactivateByExternalInput() {
      _isExternalTriggerActivate = false;
      if(_previousState) {
        if(_isMomentaryPreviously) {
          _momentary = true;
        }
      }
      else {
        off();
      }
    }
  
  
  private:
    uint8_t _relayPin;
    uint8_t _ledPin;
    uint32_t _onTime = 0;
    unsigned long _onAt = 0;
    bool _momentary = false;
    bool state = false;
    bool _previousState = false;
    bool _isMomentaryPreviously = false;
    bool _isExternalTriggerActivate = false;
};
