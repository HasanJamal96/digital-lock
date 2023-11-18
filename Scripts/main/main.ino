/*
  # Completed
    ## Keypad Functions
      - Delete user
      - Add user
      - Edit user
      - Saving data in SPIFFS
      - Reset functionality
      - Webserver
  
  # Partially Completed
    - Websocket
    - Keypad Backlight
    - Lockout settings
    
  
  
  
  # ToDo
    - Exit input
    
*/


#include <ArduinoJson.h> // https://arduinojson.org/


DynamicJsonDocument usersInfo(4096);
DynamicJsonDocument systemInfo(1024);
DynamicJsonDocument newUser(256);


#include "config.h"
#include "variables.h"
#include "buzzer.h"
#include "leds.h"
#include "relay.h"
#include "Keypad.h"
#include "memory.h"
#include "button.h"
#include "server.h"
#if (LOCK_TYPE > 0) // Condition for builds other than DL100
#include "schedule.h"
#include "rtc.h"

MyRtc         rtc;
ScheduleClass schedular;
#endif

device_state_t      deviceState = NORMAL;
programing_step_t   programStep = NONE;



Memory memory;
Buzzer buzzer      (BUZZER_PIN);
Led    exitLed     (EXIT_LED_PIN);
Relay  relay       (RELAY_PIN, RELAY_LED_PIN);
Led    keypadLed   (KEYPAD_LED_PIN);
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


void removeUser(int id) {
  usersInfo.remove(id);
  buzzer.threeShortBeeps();
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

/*
  {
    "n": "user's name 19 characters max", -> string
    "p": "user's password 19 characters max", -> string
    "l": "limited use", -> bool
    "f": "Relay function", -> uint8_t
    "o": "Momentary on time in milli seconds ", -> uint32_t
    "a": "number of times code used", -> uint16_t
    "ma" :"maximum number of time allowed to use code", -> uin16_t
  }
*/

void setDefaultNewUser() {
  newUser.clear();
  newUser["n"]  = USER_NAME;       // Username
  newUser["l"]  = LIMIT_USE;       // Limited use
  newUser["o"]  = MOMENTARY_DELAY; // Momentary on time
  newUser["f"]  = RELAY_FUNCTION;  // Relay function
  newUser["a"]  = 0;               // access counts
  newUser["ma"] = 0;               // max number of time user can access code
  newUser["s"]  = 0;               // schedule active
  newUser["e"]  = 0;               // schedule expire time in seconds
  
  
}



void populateSystemInfo() {
  const char    *dn = systemInfo["dn"];
  const char    *wn = systemInfo["wn"];
  const char    *wp = systemInfo["wp"];
  const uint8_t  cl = systemInfo["cl"];
  const bool     lo = systemInfo["lo"];
  const bool     sm = systemInfo["sm"];
  const char    *ac = systemInfo["ac"];
  
  strcpy(deviceName, dn);
  strcpy(apName, wn);
  strcpy(apPass, wp);
  maxPasscodeLength = cl;
  lockOut = lo;
  sleepMode = sm;
  strcpy(programAccessCode, ac);
}


void turnBacklight(bool on) {
  if(on) {
    backLightState = true;
    keypadLed.on();
  }
  else {
    backLightState = false;
    keypadLed.off();
  }
}

bool checkUserAllowed(int id) {
  const bool isLimited = usersInfo[id]["l"];
  bool allowed = true;
  if(isLimited) {
    const uint16_t maxAllowed = usersInfo[id]["ma"];
    uint16_t count = 0;
    count = usersInfo[id]["ma"].as<uint16_t>();
    if(maxAllowed >= count) {
      removeUser(id);
      memory.updateUsers();
      allowed = false;
    }
    else {
      count+= 1;
      usersInfo[id]["ma"] = count;
      memory.updateUsers();
      allowed = true;
    }
  }
  #if (LOCK_TYPE > 0) // Condition for builds other than DL100
    const bool isScheduled = usersInfo[id]["s"];
    if(isScheduled) {
      const unsigned long expiry = usersInfo[id]["e"];
      if(millis() >= expiry) {
        allowed = false;
        removeUser(id);
        memory.updateUsers();
      }
    }
  #endif
  return allowed;
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
              turnBacklight(true);
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
                  if(checkUserAllowed(id)) {
                    const uint8_t rFunc = usersInfo[id]["f"];
                    switch(rFunc) { // check relay functions
                      case 1: // Momentary
                        uint16_t ot;
                        ot = usersInfo[id]["o"].as<uint16_t>();
                        relay.momentaryOnFor(ot);
                        break;
                      case 2: // Latch
                        relay.on();
                        break;
                      case 3: // Unlatch
                        relay.off();
                        break;
                      case 4: // Toggle
                        relay.toggle();
                        break;
                      default:
                        break;
                    }
                  }
                }
              }
              else if(deviceState == RESET_PASSWORD) {
                memory.resetProgramAccess();
                buzzer.threeShortBeeps();
                #if (DEBUG == true)
                  Serial.printf("[Main] Reset program access code to 000000");
                #endif
              }
              else if(deviceState == RESET_ALL) {
                memory.resetUsers();
                memory.setDefaultsSettings();
                buzzer.threeShortBeeps();
                populateSystemInfo();
                #if (DEBUG == true)
                  Serial.printf("[Main] Reset everything to default");
                #endif
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
                      if(registeredUsersCount >= MAX_SUPPORTED_USERS) { // do not add user is max limit is reached
                        setDefaultNewUser();
                        buzzer.invalidBeep();
                        #if (DEBUG == true)
                          Serial.println("[Main] User limit reach");
                        #endif
                      }
                      else {
                        registeredUsersCount++;
                        memory.updateUsers();
                        buzzer.threeShortBeeps();
                      }
                      programStep = ADD_EDIT_DELETE;
                      #if (DEBUG == true)
                        Serial.println("[Main] User added");
                        serializeJsonPretty(usersInfo, Serial);
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
                        Serial.printf("[Main] User edited with id %d\n", userId);
                        serializeJsonPretty(usersInfo, Serial);
                        Serial.println();
                      #endif
                      addUser = editUser = deleteUser = false;
                    }
                    else if(deleteUser) {
                      registeredUsersCount--;
                      removeUser(userId);
                      programStep = ADD_EDIT_DELETE;
                      memory.updateUsers();
                      #if (DEBUG == true)
                        Serial.printf("[Main] User deleted with id %d\n", userId);
                        serializeJsonPretty(usersInfo, Serial);
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

void removeUserBySchedule(char *code) {
  int id = userExist(code);
  if(id != -1) {
    usersInfo.remove(id);
    memory.updateUsers();
    #if (DEBUG == true)
      Serial.println("[Main] User removed by schedular");
    #endif
  }
}


void addNewSchedule(const unsigned long ex, const char *code) {
  if(schedular.triggerOnce(ex, removeUserBySchedule, code) == 255) {
    Serial.println("[Main] Unable to add schedule");
  }
  else {
    Serial.println("[Main] Schedule added");
  }
}



#if (LOCK_TYPE > 0)
void setSchedules() {
  #if (DEBUG == true)
    Serial.println("[Main] Setting schedules");
  #endif
  
  for(int i=0; i<registeredUsersCount; i++) {
    const bool isScheduled = usersInfo[i]["s"];
    if(isScheduled) {
      const unsigned long expiry = usersInfo[i]["e"];
      if(now() >= expiry) { // remove user if time peroid is already crossed
        usersInfo.remove(i);
        memory.updateUsers();
      }
      else {
        const char *co = usersInfo[i]["p"];
        addNewSchedule(expiry, co);
      }
    }
  }
  
  #if (DEBUG == true)
    Serial.println("[Main] Schedules set");
  #endif
}
#endif


void setup() {
  #if (DEBUG == true)
    Serial.begin(BAUDRATE);
    Serial.println("[Main] Setup started");
  #endif
  
  buzzer.init();
  keypadLed.init();
  relay.begin();
  resetButton.begin();
  memory.init();
  memory.loadSystemInfo();
  memory.loadUsers();
  turnBacklight(true);
  populateSystemInfo();

  registeredUsersCount = usersInfo.size();
  
  #if (LOCK_TYPE > 0) // Condition for builds other than DL100
    rtc.init();
    setSchedules();
  #endif
  
  // Server initialization
  initServer();
  startServer();
  
  
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
    if(millis() - deviceStateChangeAt >= MODE_RESET_BACK_TO_NORMAL_AFTER) {
      changeModeToNormal();
    }
  }
  if(sleepMode && backLightState) {
    if(millis() - lastKeyPressed >= BACKLIGHT_OFF_AFTER) {
       turnBacklight(false);
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


bool updatesByServer() {
  if(addByServer) {
    addByServer = false;
    usersInfo.add(newUser);
    registeredUsersCount++;
    buzzer.threeShortBeeps();
    memory.updateUsers();
    return true;
  }
  else if(editByServer) {
    editByServer = false;
    char p[20];
    strcpy(p, newUser["p"]);
    int x = userExist(p);
    if(x != -1) {
      usersInfo[x]["n"] = newUser["n"];
      usersInfo[x]["f"] = newUser["f"];
      usersInfo[x]["p"] = newUser["p"];
      usersInfo[x]["u"] = newUser["u"];
      usersInfo[x]["o"] = newUser["o"];
      buzzer.threeShortBeeps();
      memory.updateUsers();
      return true;
    }
    else {
      return false;
    }
  }
  else if(deleteByServer) {
    deleteByServer = false;
    char p[20];
    strcpy(p, newUser["p"]);
    int x = userExist(p);
    if(x != -1) {
      registeredUsersCount--;
      removeUser(x);
      memory.updateUsers();
      return true;
    }
    else {
      return false;
    }
  }
  else if(systemByServer) {
    systemByServer = false;
    memory.updateSystemInfo();
    populateSystemInfo();
    return true;
  }
  return false;
}




void loop() {
  watchKeypad();
  deviceModeLoop();
  serverLoop();
  timedParameters();
  if(relay.status()) {
    relay.loop();
  }
  schedular.delay(100); // check all schedules
}
