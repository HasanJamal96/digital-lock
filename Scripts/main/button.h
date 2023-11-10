#include <Arduino.h>


class Button {
  public:
    Button(uint8_t pin, uint32_t dbTime=25)
        : m_pin(pin), m_dbTime(dbTime){}

    void begin() {
      pinMode(m_pin, INPUT);
      m_state = digitalRead(m_pin);
      m_time = millis();
      m_lastState = m_state;
      m_changed = false;
      m_lastChange = m_time;
    }
    bool read() {
      uint32_t ms = millis();
      bool pinVal = digitalRead(m_pin);
      if (ms - m_lastChange < m_dbTime){
          m_changed = false;
      }
      else{
          m_lastState = m_state;
          m_state = pinVal;
          m_changed = (m_state != m_lastState);
          if (m_changed) m_lastChange = ms;
      }
      m_time = ms;
      return m_state;
    }
    bool pressedFor(uint32_t ms) {
      return m_state && m_time - m_lastChange >= ms;
    }
    // bool isPressed();
    // bool isReleased();
    // bool wasPressed();
    // bool wasReleased();

  private:
    uint8_t m_pin;          // arduino pin number connected to button
    uint32_t m_dbTime;      // debounce time (ms)
    bool m_state;           // current button state, true=pressed
    bool m_lastState;       // previous button state
    bool m_changed;         // state changed since last read
    uint32_t m_time;        // time of current state (ms from millis)
    uint32_t m_lastChange;
};