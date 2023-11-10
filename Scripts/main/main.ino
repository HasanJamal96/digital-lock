/*
  # Completed
    ## Keypad Functions
      - Delete user
      - Add user
      - Edit user
  
  # Partially Completed
    - Webserver
    - Websocket
    - Reset functionality
    - Keypad Backlight
    - Lockout settings
    
  
  
  
  # ToDo
    - Exit input
    
*/


#include <ArduinoJson.h> // https://arduinojson.org/


DynamicJsonDocument usersInfo(4096);
DynamicJsonDocument systemInfo(1024);
DynamicJsonDocument newUser(100);


#include "config.h"
#include "variables.h"
#include "buzzer.h"
#include "leds.h"
#include "relay.h"
#include "Keypad.h"
#include "memory.h"
#include "button.h"
#include "server.h"


device_state_t      deviceState = NORMAL;
programing_step_t   programStep = NONE;



Memory memory;
Buzzer buzzer      (BUZZER_PIN);
Led    exitLed     (EXIT_LED_PIN);
Relay  relay       (RELAY_PIN, RELAY_LED_PIN);
Button resetButton (RESET_BTN_PIN);
Keypad keypad      (makeKeymap(keys), KEYPAD_R_PINS, KEYPAD_C_PINS, ROWS, COLS);


void valid_invalid_beeps(bool valid) {
  if(valid) {
    #if (DEBUG == true)
      Serial.println("[Main] Valid passcode");
    #endif
    buzzer.validBeep();
  }
  else {
    #if (DEBUG == true)
      Serial.println("[Main] Invalid passcode");
    #endif
    buzzer.invalidBeep();
  }
}



int userExist(char *code) {
  if(passcodeIndex == maxPasscodeLength) {
    for(uint16_t i=0; i<registeredUsersCount; i++) {
      const char *p = usersInfo[i]["p"];
      if(strcmp(p, code) == 0) {
        return i;
      }
    }
  }
  return -1;
}




int matchUsers(char *code) {
  int x = userExist(code);
  if(x != -1) {
    valid_invalid_beeps(true);
  }
  else
    valid_invalid_beeps(false);
  return x;
}


void matchAdminPasscode() {
  if(passcodeIndex == PROGRAM_ACCESS_CODE_LEN) {
    if(strcmp(passcode, programAccessCode) == 0) {
      deviceState = PROGRAMING;
      programStep = ADD_EDIT_DELETE;
      currentPassLength = maxPasscodeLength;
      buzzer.validBeep();
      #if (DEBUG == true)
        Serial.println("[Main] Device mode is changed to PROGRAMING");
      #endif
      return;
    }
  }
  valid_invalid_beeps(false);
}




void emptyPassCode() {
  for(uint8_t i=0; i<MAX_CODE_LENGTH+1; i++) {
    passcode[i] = '\0';
  }
  passcodeIndex = 0;
}



void populatePasscode(char k) {
  if(passcodeIndex >= currentPassLength) {
    emptyPassCode();
  }
  passcode[passcodeIndex] = k;
  passcodeIndex++;
}

void setDefaultNewUser() {
  newUser.clear();
  newUser["n"] = USER_NAME;       // Usrename
  newUser["u"] = LIMIT_USE;       // Limited use
  newUser["o"] = MOMENTARY_DELAY; // Momentary on time
  newUser["f"] = RELAY_FUNCTION;  // Relay function
}



void watchKeypad() {
  if (keypad.getKeys()) {
    for(uint8_t i=0; i<LIST_MAX; i++) {
      if(keypad.key[i].stateChanged ) {
        const char k = keypad.key[i].kchar;
        switch (keypad.key[i].kstate) {
          case PRESSED:
            buzzer.shortBeep();
            lastKeyPressed = millis();
            if(!backLightState) {
              // turn on Backlight
            }
            if(k > 47 && k < 58) {
              if(deviceState != PROGRAMING) {
                populatePasscode(k);
              }
              else {
                if(programStep == ADD_EDIT_DELETE) {
                  if(k == 49) {
                    addUser    = true;
                    editUser   = false;
                    deleteUser = false;
                  }
                  else if(k == 50) {
                    addUser    = false;
                    editUser   = true;
                    deleteUser = false;
                    emptyPassCode();
                  }
                  else if(k == 51) {
                    addUser    = false;
                    editUser   = false;
                    deleteUser = true;
                    emptyPassCode();
                  }
                }
                else if(programStep == SELECT_FUNCTIONS) {
                  if(k > 48 && k < 52) {
                    if(!deleteUser) {
                      if(k == 49) {
                        selectedFunction = 1; // Momentary
                        functionConfirmed = true;
                      }
                      else if(k == 50) {
                        selectedFunction = 4; // Toggle
                        functionConfirmed = true;
                      }
                    }
                    else if(deleteUser) {
                      if(k == 51) {
                        functionConfirmed = true;
                      }
                    }
                  }
                  else {
                    functionConfirmed = false;
                  }
                }
                else { // ADD, EDIT, DELETE, NEW_PASSWORD
                  populatePasscode(k);
                }
              }
            }
            else if(k == '#') {
              enterPressedAt = millis();
              enterState = true;
            }
            else if(k == '*') {
              cancelPressedAt = millis();
              cancelState = true;
            }
            break;
          case RELEASED:
            if(k == '#') {
              enterState = false;
              if(deviceState == NORMAL) {
                #if (DEBUG == true)
                  Serial.printf("[Main] Passcode entered: %s\n", passcode);
                #endif
                int id = matchUsers(passcode);
                if(id != -1) {
                  // Run relay function
                }
              }
              else if(deviceState == RESET_PASSWORD) {
                // Reset program password
                buzzer.threeShortBeeps();
              }
              else if(deviceState == RESET_ALL) {
                // Reset everything 
                buzzer.threeShortBeeps();
              }
              else if(deviceState == WAIT_PROGRAMING_CODE) {
                if(passcodeIndex == PROGRAM_ACCESS_CODE_LEN) {
                  matchAdminPasscode();
                }
              }
////////////////////////////////////////////////////  PROGRAMMING
              else {
////////////////////////////////////////////////////  ADD_EDIT_DELETE
                if(programStep == ADD_EDIT_DELETE) {
                  if(addUser) {
                    programStep = ADD;
                    #if (DEBUG == true)
                      Serial.println("[Main] Adding new user, Add new password...");
                    #endif
                  }
                  else if(editUser) {
                    programStep = EDIT;
                    #if (DEBUG == true)
                      Serial.println("[Main] Editing user, Please verify...");
                    #endif
                  }
                  else if(deleteUser) {
                    programStep = DELETE;
                    #if (DEBUG == true)
                      Serial.println("[Main] Deleting user, Please verify...");
                    #endif
                  }
                  if(addUser || editUser || deleteUser) {
                    emptyPassCode();
                    setDefaultNewUser();
                    userConfirmed = functionConfirmed = false;
                  }
                }
////////////////////////////////////////////////////  ADD
                else if(programStep == ADD) {
                  if(passcodeIndex != maxPasscodeLength) {
                    #if (DEBUG == true)
                      Serial.println("[Main] Invalid password");
                    #endif
                    buzzer.invalidBeep();
                  }
                  else {
                    int id = userExist(passcode);
                    if(id != -1) {
                      #if (DEBUG == true)
                        Serial.println("[Main] Password already exist");
                      #endif
                      buzzer.invalidBeep();
                    }
                    else {
                      programStep = SELECT_FUNCTIONS;
                      newUser["p"] = passcode;
                      #if (DEBUG == true)
                        Serial.println("[Main] Select relay function");
                      #endif
                    }
                  }
                }
////////////////////////////////////////////////////  EDIT / DELETE
                else if(programStep == EDIT || programStep == DELETE) {
                  if(!userConfirmed) {
                    if(passcodeIndex < maxPasscodeLength) {
                      buzzer.invalidBeep();
                    }
                    else {
                      userId = matchUsers(passcode);
                      if(userId != -1) {
                        emptyPassCode();
                        userConfirmed = true;
                        if(programStep == EDIT) {
                          programStep = NEW_PASSWORD;
                          #if (DEBUG == true)
                            Serial.println("[Main] Enter new password");
                          #endif
                        }
                        else {
                          programStep = SELECT_FUNCTIONS;
                          #if (DEBUG == true)
                            Serial.println("[Main] Press 3 than # to verify");
                          #endif
                        }
                      }
                    }
                  }
                }
////////////////////////////////////////////////////  NEW_PASSWORD
                else if(programStep == NEW_PASSWORD) {
                  if(editUser) {
                    if(passcodeIndex < maxPasscodeLength) {
                      buzzer.invalidBeep();
                    }
                    else {
                      newUser["p"] = passcode;
                      programStep = SELECT_FUNCTIONS;
                      #if (DEBUG == true)
                        Serial.println("[Main] Select relay function");
                      #endif
                    }
                  }
                }
////////////////////////////////////////////////////  SELECT_FUNCTIONS
                else if(programStep == SELECT_FUNCTIONS) {
                  if(functionConfirmed) {
                    if(addUser) {
                      newUser["f"] = selectedFunction;
                      usersInfo.add(newUser);
                      registeredUsersCount++;
                      buzzer.threeShortBeeps();
                      programStep = ADD_EDIT_DELETE;
                      memory.updateUsers();
                      #if (DEBUG == true)
                        Serial.println("[Main] User added");
                        serializeJson(usersInfo, Serial);
                        Serial.println();
                      #endif
                      addUser = editUser = deleteUser = false;
                    }
                    else if(editUser) {
                      usersInfo[userId]["f"] = selectedFunction;
                      usersInfo[userId]["p"] = newUser["p"];
                      buzzer.threeShortBeeps();
                      programStep = ADD_EDIT_DELETE;
                      memory.updateUsers();
                      #if (DEBUG == true)
                        Serial.printf("User Edited with id %d\n", userId);
                        serializeJson(usersInfo, Serial);
                        Serial.println();
                      #endif
                      addUser = editUser = deleteUser = false;
                    }
                    else if(deleteUser) {
                      registeredUsersCount--;
                      usersInfo.remove(userId);
                      buzzer.threeShortBeeps();
                      programStep = ADD_EDIT_DELETE;
                      memory.updateUsers();
                      #if (DEBUG == true)
                        Serial.printf("User deleted with id %d\n", userId);
                        serializeJson(usersInfo, Serial);
                        Serial.println();
                      #endif
                      addUser = editUser = deleteUser = false;
                    }
                  }
                  else {
                    Serial.println("[Main] Fcuntion not selected");
                  }
                }
              }
              emptyPassCode();
            }
            else if(k == '*') {
              cancelState = false;
            }
            break;
          default:
            break;
        }
      }
    }
  }
  if(millis() - lastKeyPressed >= KEY_RESET_DELAY) {
    emptyPassCode();
  }
}


void populateSystemInfo() {
  const char *dn = systemInfo["dn"];
  const char *wn = systemInfo["wn"];
  const char *wp = systemInfo["wp"];
  const uint8_t  cl = systemInfo["cl"];
  const bool lo = systemInfo["lo"];
  const bool sm = systemInfo["sm"];
  const char *ac = systemInfo["ac"];
  
  strcpy(deviceName, dn);
  strcpy(apName, wn);
  strcpy(apPass, wp);
  maxPasscodeLength = cl;
  lockOut = lo;
  sleepMode = sm;
  strcpy(programAccessCode, ac);
}


void setup() {
  #if (DEBUG == true)
    Serial.begin(BAUDRATE);
    Serial.println("[Main] Setup started");
  #endif
  
  
  deviceState = WAIT_PROGRAMING_CODE;
  currentPassLength = PROGRAM_ACCESS_CODE_LEN;
  
  buzzer.init();
  relay.begin();
  resetButton.begin();
  memory.init();
  memory.loadSystemInfo();
  memory.loadUsers();
  populateSystemInfo();
  
  
  
  
  // newUser["p"] = "1234";
  // usersInfo.add(newUser);
  // newUser["p"] = "2486";
  // usersInfo.add(newUser);
  // registeredUsersCount = 2;
  
  // memory.updateUsers();
  
  
  #if (DEBUG == true)
    Serial.println("[Main] Setup complete");
  #endif
}


void changeModeToNormal() {
  deviceState = NORMAL;
  programStep = NONE;
  currentPassLength = maxPasscodeLength;
  buzzer.twoNormalBeeps();
  addUser = editUser = deleteUser = false;
  #if (DEBUG == true)
    Serial.println("[Main] Device mode is changed to NORMAL");
  #endif
}



void deviceModeLoop() {
  if(cancelState) {
    if(millis() - cancelPressedAt >= EXIT_ENTER_PROGRAM_MODE) {
      if(deviceState == NORMAL) {
        deviceStateChangeAt = millis();
        deviceState = WAIT_PROGRAMING_CODE;
        currentPassLength = PROGRAM_ACCESS_CODE_LEN;
        buzzer.twoNormalBeeps();
        #if (DEBUG == true)
          Serial.println("[Main] Device mode is changed to WAIT_PROGRAMING_CODE");
        #endif
      }
      else {
        changeModeToNormal();
      }
      cancelState = false;
    }
  }
}


void timedParameters() {
  if(deviceState != NORMAL) {
    if(millis() - deviceStateChangeAt >= 240000) {
      changeModeToNormal();
    }
  }
  if(sleepMode && backLightState) {
    if(millis() - lastKeyPressed >= 30) {
       // turn off backlight
       backLightState = false;
    }
  }
  resetButton.read();
  if(cancelState && (millis() - cancelPressedAt >= 10000) && resetButton.pressedFor(10000)) {
    buzzer.twoShortBeeps();
    deviceState = RESET_PASSWORD;
  }
  if(cancelState && enterState) {
    if((millis() - cancelPressedAt >= 10000) && (millis() - enterPressedAt >= 10000)) {
      deviceState = RESET_ALL;
      buzzer.twoShortBeeps();
    }
  }
}




void loop() {
  watchKeypad();
  deviceModeLoop();
  websocketLoop();
  timedParameters();
}
