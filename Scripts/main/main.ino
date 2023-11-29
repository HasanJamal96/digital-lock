/*
  # External Libraries used in this project, have to install manually
    (1.1.1)  AsyncTCP.h              https://github.com/me-no-dev/AsyncTCP
    (6.21.2) ArduinoJson.h           https://arduinojson.org/
    (1.2.3)  ESPAsyncWebServer.h     https://github.com/me-no-dev/ESPAsyncWebServer
    (2.0.2)  Ethernet.h              https://github.com/arduino-libraries/Ethernet
    (1.6.1)  TimeLib.h               https://github.com/PaulStoffregen/Time
    (2.1.3)  RTClib.h                https://github.com/adafruit/RTClib
    (1.14.5) Adafruit_BusIO          https://github.com/adafruit/Adafruit_BusIO/tree/master
*/


#include <ArduinoJson.h>

StaticJsonDocument<8192> usersInfo;
StaticJsonDocument<512> systemInfo;
StaticJsonDocument<256> newUser;



#include "config.h"
#include "variables.h"
#include "buzzer.h"
#include "leds.h"
#include "relay.h"
#include "Keypad.h"
#include "memory.h"
#include "button.h"

device_state_t      deviceState = NORMAL;
programing_step_t   programStep = NONE;



Memory memory;
Buzzer buzzer      (BUZZER_PIN);
Led    exitLed     (EXIT_LED_PIN);
Button resetButton (RESET_BTN_PIN);
Led    keypadLed   (KEYPAD_LED_PIN);
Relay  relay       (RELAY_PIN, RELAY_LED_PIN);
Keypad keypad      (makeKeymap(keys), KEYPAD_R_PINS, KEYPAD_C_PINS, ROWS, COLS);

#if (LOCK_TYPE < 2) // For DL100 and DL1500 Timer
  #include "basic_server.h"
#else
  #include "advance_server.h"
#endif

#if (LOCK_TYPE > 0) // For other than DL100
  #include "schedule.h"
  #include "rtc.h"
  
  MyRtc         rtc;
  ScheduleClass schedular;
#endif



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



int userExist(char *code, bool x = false) {
  if(!code)
    return -1;
  if(passcodeIndex == maxPasscodeLength || x) {
    for(uint16_t i=0; i<registeredUsersCount; i++) {
      const char *up = usersInfo[i]["p"];
      if(up) {
        if(strcmp(up, code) == 0) {
          return i;
        }
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
    "n" : "user's name 19 characters max", -> string
    "p" : "user's password 19 characters max", -> string
    "l" : "limited use", -> bool
    "f" : "Relay function", -> uint8_t
    "o" : "Momentary on time in milli seconds ", -> uint32_t
    "a" : "number of times code used", -> uint16_t
    "ma": "maximum number of time allowed to use code", -> uin16_t
    "s" : "guest user", -> 0,1
    "e" : "time in seconds" -> unsigned long 
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
  #if(LOCK_TYPE > 0)               // Condition for builds other than DL100
    newUser["s"]  = 0;             // schedule active
    newUser["e"]  = 0;             // schedule expire time in seconds
  #endif
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
    count = usersInfo[id]["a"].as<uint16_t>();
    if(count >= maxAllowed) {
      removeUser(id);
      memory.updateUsers();
      allowed = false;
    }
    else {
      count+= 1;
      usersInfo[id]["a"] = count;
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
              if(!enterState)
                break;
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
                deviceState = NORMAL;
                memory.resetProgramAccess();
                buzzer.threeShortBeeps();
                #if (DEBUG == true)
                  Serial.printf("[Main] Reset program access code to 000000\n");
                #endif
              }
              else if(deviceState == RESET_ALL) {
                deviceState = NORMAL;
                memory.resetUsers();
                memory.setDefaultsSettings();
                buzzer.threeShortBeeps();
                populateSystemInfo();
                #if (DEBUG == true)
                  Serial.printf("[Main] Reset everything to default\n");
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
                      if(registeredUsersCount >= MAX_SUPPORTED_USERS) { // do not add user is max limit is reached
                        setDefaultNewUser();
                        buzzer.invalidBeep();
                        #if (DEBUG == true)
                          Serial.println("[Main] User limit reach");
                        #endif
                      }
                      else {
                        usersInfo.add(newUser);
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
  int id = userExist(code, true);
  if(id != -1) {
    usersInfo.remove(id);
    memory.updateUsers();
    #if (DEBUG == true)
      Serial.println("[Schedular] User removed");
    #endif
  }
  #if (DEBUG == true)
    else {
        Serial.println("[Schedular] User not exist");
    }
  #endif
}

#if (LOCK_TYPE > 0)
void addNewSchedule(const unsigned long ex, const char *code) {
  if(schedular.triggerOnce(ex, removeUserBySchedule, code) == 255) {
    #if (DEBUG == true)
      Serial.println("[Schedular] Unable to add schedule");
    #endif
  }
  #if (DEBUG == true)
    else {
      Serial.println("[Schedular] Schedule added");
    }
  #endif
}
void setSchedules() {
  #if (DEBUG == true)
    Serial.println("[Main] Setting schedules");
  #endif
  for(int i=0; i<registeredUsersCount; i++) {
    const bool isScheduled = usersInfo[i]["s"];
    if(isScheduled) {
      const unsigned long expiry = usersInfo[i]["e"];
      if(now() >= expiry) { // remove user if time peroid is already passed
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

  registeredUsersCount = usersInfo.size();
  
  
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
  if(cancelState && !enterState) {
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
  bool btnState = resetButton.read();
  if(sleepMode && backLightState) {
    if(millis() - lastKeyPressed >= BACKLIGHT_OFF_AFTER) {
       turnBacklight(false);
    }
  }
  if(btnState) {
    if(cancelState && (millis() - cancelPressedAt >= 10000) && resetButton.pressedFor(10000)) {
      buzzer.twoShortBeeps();
      deviceState = RESET_PASSWORD;
      cancelState = false;
    }
  }
  else {
    if(deviceState != RESET_PASSWORD && deviceState != RESET_ALL) {
      deviceModeLoop();
      if(deviceState != NORMAL) {
        if(millis() - lastKeyPressed >= MODE_RESET_BACK_TO_NORMAL_AFTER) {
          changeModeToNormal();
        }
      }
      if(cancelState && enterState) {
        if((millis() - cancelPressedAt >= 10000) && (millis() - enterPressedAt >= 10000)) {
          deviceState = RESET_ALL;
          buzzer.twoShortBeeps();
          enterState = cancelState = false;
        }
      }
    }
  }
}


bool updatesByServer() {
  if(addByServer) {
    addByServer = false;
    char pas[7];
    strcpy(pas, newUser["p"]);
    int x = userExist(pas, true);
    if(x == -1) {
      usersInfo.add(newUser);
      registeredUsersCount++;
      buzzer.threeShortBeeps();
      memory.updateUsers();
      return true;
    }
    else {
      return false;
    }
  }
  else if(editByServer) {
    editByServer = false;
    char pas[7];
    strcpy(pas, newUser["p"]);
    int x = userExist(pas, true);
    if(x != -1) {
      const char *np = newUser["np"];
      newUser.remove("np");
      usersInfo[x] = newUser;
      usersInfo[x]["p"] = np;
//      usersInfo[x]["n"] = newUser["n"];
//      usersInfo[x]["f"] = newUser["f"];
//      usersInfo[x]["p"] = newUser["np"];
//      usersInfo[x]["o"] = newUser["o"];
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
    char pas[7];
    strcpy(pas, newUser["p"]);
    int x = userExist(pas, true);
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


void relayFunctionsForServer(uint8_t x) {
  switch(x) {
    case 0: // off
      relay.off();
      break;
    case 1: // on
      relay.on();
      break;
  }
}



void loop() {
  watchKeypad();
  serverLoop();
  timedParameters();
  if(relay.status()) {
    relay.loop();
  }
  #if(LOCK_TYPE > 0)
    schedular.delay(100); // check all schedules
  #endif
}
