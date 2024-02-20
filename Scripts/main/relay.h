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


    void updateState(uint8_t stateToSet, uint16_t setOnTime = 0) {
      if(stateToSet == R_HOLD || stateToSet == R_LATCH || stateToSet == R_OPEN) {
        on();
        rMode = stateToSet;
      }
      else if(stateToSet == R_UNLATCH || stateToSet == R_CLOSE) {
        off();
        rMode = stateToSet;
      }
      else if(stateToSet == R_MOMENTARY) {
        momentaryOnFor(setOnTime);
        rMode = R_MOMENTARY;
      }
      else if(stateToSet == R_TOGGLE) {
        toggle();
        rMode = R_TOGGLE;
      }
      if(EEPROM.read(ACTIVE_SCHEDULED_LOCATION) != 255) {
        EEPROM.write(ACTIVE_SCHEDULED_LOCATION, 255);
        EEPROM.commit();
      }
    }


    bool setState(uint8_t stateToSet, uint16_t setOnTime = 0) {
      if(_isExternalTriggerActivate)
        return false;
      if(stateToSet == R_OPEN || stateToSet == R_CLOSE || stateToSet == R_MOMENTARY || stateToSet == R_LATCH) {
        if(rMode == R_HOLD || (rMode == R_TOGGLE && state)) {
          return false;
        }
      }
      updateState(stateToSet, setOnTime);
      return true;
    }


    bool status() {
      return state;
    }


    uint8_t getMode() {
      return rMode;
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
      _previousState = state;
      _isMomentaryPreviously = _momentary;
      on();
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
    uint8_t rMode = R_NONE;
};
