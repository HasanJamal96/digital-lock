#define RELAY_ON  HIGH
#define RELAY_OFF LOW
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
      digitalWrite(_relayPin, RELAY_ON);
      digitalWrite(_ledPin, LED_ON);
      state = true;
    }
    
    void off() {
      digitalWrite(_relayPin, RELAY_OFF);
      digitalWrite(_ledPin, LED_OFF);
      state = false;
    }
    
    bool status() {
      return state;
    }
  
  
  private:
    uint8_t _relayPin;
    uint8_t _ledPin;
    bool state = false;
};