#ifndef Keypadlib_KEY_H_
#define Keypadlib_KEY_H_

#include <Arduino.h>

#define OPEN LOW
#define CLOSED HIGH

typedef unsigned int uint;
typedef enum{ IDLE, PRESSED, HOLD, RELEASED } KeyState;

const char NO_KEY = '\0';

class Key {
public:
	char kchar;
	int kcode;
	KeyState kstate;
	boolean stateChanged;

	Key() {
    kchar = NO_KEY;
    kstate = IDLE;
    stateChanged = false;
  }
  
  
	Key(char userKeyChar) {
    kchar = userKeyChar;
    kcode = -1;
    kstate = IDLE;
    stateChanged = false;
  }
  
  
	void key_update(char userKeyChar, KeyState userState, boolean userStatus) {
    kchar = userKeyChar;
    kstate = userState;
    stateChanged = userStatus;
  }

};

#endif