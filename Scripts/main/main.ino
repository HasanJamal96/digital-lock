/*
  # External Libraries used in this project, have to install manually
    (1.1.1)  AsyncTCP.h              https://github.com/me-no-dev/AsyncTCP
    (6.21.2) ArduinoJson.h           https://arduinojson.org/
    (1.2.3)  ESPAsyncWebServer.h     https://github.com/me-no-dev/ESPAsyncWebServer
    (2.0.2)  Ethernet.h              https://github.com/arduino-libraries/Ethernet
    (1.6.1)  TimeLib.h               https://github.com/PaulStoffregen/Time
    (2.1.3)  RTClib.h                https://github.com/adafruit/RTClib
    (1.14.5) Adafruit_BusIO          https://github.com/adafruit/Adafruit_BusIO/tree/master
    (1.6.0)  mWebSockets
      have to comment these two files completely
        (WebSocketServer.h, WebSocketServer.cpp)
*/


#include <ArduinoJson.h>

StaticJsonDocument<16384> usersInfo;
StaticJsonDocument<512>   systemInfo;
StaticJsonDocument<512>   newUser;
StaticJsonDocument<2048> history;

 /*
  {
    "t":"millis", -> time in millis
    "n":"name" -> user name if user, schedule name if schedule,
    "c":"1111", -> if user
    "a":"0/1", -> access granted or not, 0 not granted, 1 granted
  }
 */



#include "config.h"
#include "EEPROM.h"
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
Button exitInput   (EXIT_INPUT_PIN);
Button resetButton (RESET_BTN_PIN);
Led    keypadLed   (KEYPAD_LED_PIN);
Relay  relay       (RELAY_PIN, RELAY_LED_PIN);
Keypad keypad      (makeKeymap(keys), KEYPAD_R_PINS, KEYPAD_C_PINS, ROWS, COLS);

#if (LOCK_TYPE > 0) // For other than DL100
  #include "schedule.h"
  #include "rtc.h"

  MyRtc         rtc;
  ScheduleManager schedule;
#endif


#if (LOCK_TYPE < 2)
  #include "basic_server.h"  // For DL100 and DL1500 Timer
#else
  #include "advance_server.h"  // For other than DL100 and DL1500 Timer
#endif


void addHistory(const char *name, char *code, const uint8_t allowed) {
  uint8_t len = history.size();
  if(len > MAX_HISTORY) {
    history.remove(0);
  }

  DynamicJsonDocument doc(256);
  doc["t"] = now();
  doc["n"] = name;
  doc["p"] = code;
  if(allowed)
    doc["a"] = 1;
  else
    doc["a"] = 0;
  
  history.add(doc);
  // #if(DEBUG == true)
  //   serializeJson(history, Serial);
  //   Serial.println();
  // #endif
  history.garbageCollect();
  memory.updateHistory();
}


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


void removeUser(int id, bool doBuzz = true) {
  usersInfo.remove(id);
  usersInfo.garbageCollect();
  if(doBuzz)
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
    "p" : "user's password 6 characters max", -> string
    "f" : "Relay function", -> uint8_t
    "o" : "Momentary on time in seconds ", -> uint32_t
    "l" : "limited use", -> bool
    "a" : "number of times code used", -> uint16_t
    "ma": "maximum number of time allowed to use code", -> uin16_t
    "s" : "guest user", -> 0,1
    "e" : "time in seconds", -> unsigned long
    "np" : "new password"
  }

  // adding guest user
  {
    "n" : "user's name 19 characters max", -> string
    "p" : "user's password 6 characters max", -> string
    "f" : "Relay function", -> uint8_t
    "o" : "Momentary on time in seconds ", -> uint32_t
    "l" : "limited use", -> bool
    "a" : "number of times code used", -> uint16_t
    "ma": "maximum number of time allowed to use code", -> uin16_t
    "s" : "guest user", -> 0,1
    "sd" : "start date" -> unsigned long, value cannot be zero
    "ed" : "end date" -> unsigned long, value can be zero (no expiry)
    "st" : "start time in seconds 24-hours" -> unsigned long
    "et" : "end time in seconds 24-hours" -> unsigned long
    "wd" : "weekdays " -> uint8_t each bit from right to left is week day startting from sunday example (Monday, Wednesday, Friday) = 00101010
  }

  // Relay Schedules

  {
    "sd" : "start date" -> unsigned long, value cannot be zero
    "ed" : "end date" -> unsigned long, value can be zero (no expiry)
    "st" : "start time in seconds 24-hours" -> unsigned long
    "et" : "end time in seconds 24-hours" -> unsigned long
    "wd" : "weekdays " -> uint8_t each bit from right to left is week day startting from sunday example (Monday, Wednesday, Friday) = 00101010
    "sf" : "relay function at start time"
    "ef" : "relay function at end time"
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

void copyData(char *var1, const char *var2) {
  if(strlen(var2) > 0) {
    strcpy(var1, var2);
  }
}



void populateSystemInfo() {
  const char    *dn = systemInfo["dn"];
  const char    *wn = systemInfo["wn"];
  const char    *wp = systemInfo["wp"];
  const uint8_t  cl = systemInfo["cl"];
  const bool     lo = systemInfo["lo"];
  const bool     sm = systemInfo["sm"];
  const char    *ac = systemInfo["ac"];

  copyData(deviceName, dn);
  copyData(apName, wn);
  copyData(apPass, wp);
  maxPasscodeLength = cl;
  lockOut = lo;
  sleepMode = sm;
  strcpy(programAccessCode, ac);
}


void turnBacklight(bool on) {
  if(on) {
    if(!backLightState) {
      backLightState = true;
      keypadLed.on();
    }
  }
  else {
    if(backLightState) {
      backLightState = false;
      keypadLed.off();
    }
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
      unsigned long sd = usersInfo[id]["sd"];
      unsigned long ed = usersInfo[id]["ed"];
      unsigned long st = usersInfo[id]["st"];
      unsigned long et = usersInfo[id]["et"];
      uint8_t       wd = usersInfo[id]["wd"];
      if(!schedule.isUserInLimits(sd, ed, st, et, wd)) {
        allowed = false;
      }
    }
  #endif
  return allowed;
}


void runRelayFunction(uint8_t x, const char *name = "", uint8_t index = 0) {
  char c[2];
  sprintf(c, "%d", index);
  EEPROM.write(ACTIVE_SCHEDULED_LOCATION, index);
  EEPROM.commit();
  addHistory(name, c, 1);
  switch(x) {
    case 2: // Latch
      relay.on();
      break;
    case 3: // Unlatch
      relay.off();
      break;
    // case 4: // Toggle
    //   relay.toggle();
    //   break;
    default:
      break;
  }
}

bool checkIfDeviceIsLocked() {
  if(lockOut) {
    if(lockTheDevice) {
      if(millis() - lockOutStartTime < LOCK_WAIT_TIME) {
        return true;
      }
      else {
        consectiveInvalidEntries = 0;
        lockTheDevice = false;
      }
    }
  }
  return false;
}




void watchKeypad() {
  if(checkIfDeviceIsLocked()) {
    return;
  }
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
            if(k > 47 && k < 58) {
              if(deviceState == NORMAL) {
                if(strlen(passcode) >= maxPasscodeLength) {
                  #if (DEBUG == true)
                    Serial.printf("[Main] Passcode entered: %s\n", passcode);
                  #endif
                  int id = userExist(passcode, false);
                  if(id != -1) {
                    consectiveInvalidEntries = 0;
                    const char *un = usersInfo[id]["n"];
                    if(checkUserAllowed(id)) {
                      const uint8_t rFunc = usersInfo[id]["f"];
                      if(rFunc >= 0 && rFunc < 5) {
                        bool granted = false;
                        switch(rFunc) { // check relay functions
                          case 1: // Momentary
                            uint16_t ot;
                            ot = usersInfo[id]["o"].as<uint16_t>();
                            granted = relay.setState(R_MOMENTARY, ot);
                            // relay.momentaryOnFor(ot);
                            break;
                          case 2: // Latch
                            granted = relay.setState(R_LATCH);
                            // relay.on();
                            break;
                          case 3: // Unlatch
                            granted = relay.setState(R_UNLATCH);
                            // relay.off();
                            break;
                          case 4: // Toggle
                            granted = relay.setState(R_TOGGLE);
                            // relay.toggle();
                            break;
                          default:
                            break;
                        }
                        if(granted) {
                          valid_invalid_beeps(true);
                          addHistory(un, passcode, 1);
                        }
                        else {
                          addHistory(un, passcode, 0);
                          valid_invalid_beeps(false);
                        }
                      }
                    }
                    else {
                      addHistory(un, passcode, 0);
                      valid_invalid_beeps(false);
                    }
                  }
                  else {
                    if(lockOut) {
                      consectiveInvalidEntries += 1;
                      if(consectiveInvalidEntries >= MAX_INVALID_ATTEMPTS) {
                        lockTheDevice = true;
                        lockOutStartTime = millis();
                      }
                    }
                    valid_invalid_beeps(false);
                  }
                }
              }
            }
            else if(k == '#') {
              if(!enterState)
                break;
              enterState = false;
              if(deviceState == RESET_PASSWORD) {
                deviceState = NORMAL;
                memory.resetProgramAccess();
                strcpy(programAccessCode, PROG_PASSWORD);
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


void setup() {
  #if (DEBUG == true)
    Serial.begin(BAUDRATE);
    Serial.println("[Main] Setup started");
  #endif

  exitLed.init();
  exitInput.begin();

  buzzer.init();
  keypadLed.init();
  relay.begin();
  resetButton.begin();
  memory.init();
  memory.loadSystemInfo();
  memory.loadUsers();
  memory.loadHistory();
  turnBacklight(true);
  populateSystemInfo();

  registeredUsersCount = usersInfo.size();

  #if (LOCK_TYPE > 0) // Condition for builds other than DL100
    rtc.init();
    memory.loadRealySchedule(&schedule);
    schedule.setCallbackFunction(runRelayFunction);

    EEPROM.begin(10);
    uint8_t magicNumber = EEPROM.read(MAGIC_NUMBER_LOCATION);
    if(magicNumber != MAGIC_NUMBER) {
      EEPROM.write(MAGIC_NUMBER_LOCATION, MAGIC_NUMBER);
      EEPROM.write(ACTIVE_SCHEDULED_LOCATION, 255);
      EEPROM.commit();
    }
    else {
      uint8_t savedScheduledIndex = EEPROM.read(ACTIVE_SCHEDULED_LOCATION);
      if(savedScheduledIndex != INVALID_SCHEDULE) {
        char buf[150];
        schedule.getSpecificSchedule(buf, savedScheduledIndex);
        if(strlen(buf) > 10) {
          DynamicJsonDocument temp(200);
          DeserializationError error = deserializeJson(temp, buf);
          if(error) {
            #if(DEBUG == true && DEBUG_MEMORY == true)
              Serial.print("[SPIFFS] Failed to parse last activated realy schedule: ");
              Serial.println(error.f_str());
            #endif
            EEPROM.write(ACTIVE_SCHEDULED_LOCATION, 255);
            EEPROM.commit();
          }
          else {
            delay(500);
            unsigned long sd = temp["sd"];
            unsigned long ed = temp["ed"];
            unsigned long st = temp["st"];
            unsigned long et = temp["et"];
            uint8_t wd = temp["wd"];
            uint8_t sf = temp["sf"];
            if(schedule.isUserInLimits(sd, ed, st, et, wd)) {
              #if (DEBUG == true)
                Serial.println("[Main] Schedule is in limits");
              #endif
              switch(sf) {
                case 2:
                  relay.on();
                  break;
                case 3:
                  relay.off();
                  break;
                default:
                  EEPROM.write(ACTIVE_SCHEDULED_LOCATION, 255);
                  EEPROM.commit();
              }
            }
            else {
              EEPROM.write(ACTIVE_SCHEDULED_LOCATION, 255);
              EEPROM.commit();
            }
          }
        }
        else {
          EEPROM.write(ACTIVE_SCHEDULED_LOCATION, 255);
          EEPROM.commit();
        }
      }
    }
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
        emptyPassCode();
        buzzer.twoNormalBeeps();
        #if (DEBUG == true)
          Serial.println("[Main] Device mode is changed to WAIT_PROGRAMING_CODE");
        #endif
      }
      else {
        emptyPassCode();
        changeModeToNormal();
      }
      cancelState = false;
    }
  }
}


void timedParameters() {
  bool btnState = resetButton.read();
  if(!sleepMode) {
    turnBacklight(true);
  }
  else if(backLightState) {
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

void removeExtraKeys(bool limit, bool guest) {
  if(!limit) {
    newUser.remove("a");
    newUser.remove("ma");
  }
  if(!guest) {
    newUser.remove("sd");
    newUser.remove("ed");
    newUser.remove("st");
    newUser.remove("et");
    newUser.remove("wd");
  }
  newUser.remove("np");
}


uint8_t updatesByServer() {
  if(addByServer) {
    addByServer = false;
    char pas[7];
    strcpy(pas, newUser["p"]);
    int x = userExist(pas, true);
    if(x == -1) {
      const bool li = newUser["l"];
      const bool gu = newUser["s"];
      removeExtraKeys(li, gu);
      usersInfo.add(newUser);
      registeredUsersCount++;
      buzzer.threeShortBeeps();
      memory.updateUsers();
      return 1;
    }
    else {
      return 0;
    }
  }
  else if(editByServer) {
    editByServer = false;
    char pas[7];
    strcpy(pas, newUser["p"]);
    int x = userExist(pas, true); // check is user exist
    if(x != -1) {
      const char *np = newUser["np"];
      newUser.remove("np");
      strcpy(pas, np);
      int y = userExist(pas, true); // check if new password already exist
      if(y == -1 || x == y) { // new password not exist
        const bool li = newUser["l"];
        const bool gu = newUser["s"];
        removeExtraKeys(li, gu);
        usersInfo[x] = newUser;
        if(y == -1)
          usersInfo[x]["p"] = np;
        buzzer.threeShortBeeps();
        memory.updateUsers();
//        serializeJson(usersInfo, Serial);
//        Serial.println();
        return 1; 
      }
      else {
        #if (DEBUG == true)
          Serial.println("[Main] New password already exist");
        #endif
        return 2;
      }
    }
    #if (DEBUG == true)
      else {
        Serial.println("[Main] User not exist");
      }
    #endif
    return 0;
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
      return 1;
    }
    else {
      return 0;
    }
  }
  else if(systemByServer) {
    systemByServer = false;
    memory.updateSystemInfo();
    populateSystemInfo();
    return 1;
  }
  return 0;
}



void relayFunctionsForServer(uint8_t x) {
  if(x == 0)
    relay.off();
  else
    relay.on();
}

#if (DEBUG == true)
uint32_t lastHespPrint = 0;
void printFreeRam() {
  if(millis() - lastHespPrint >= 2000) {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    lastHespPrint = millis();
  }
}
#endif


void loop() {
  watchKeypad();
  serverLoop();
  timedParameters();
  if(relay.status()) {
    relay.loop();
  }
  #if(LOCK_TYPE > 0)
    schedule.service(); // check all schedules
    if(schedule.scheduleChanged()) {
      char buf[1024];
      schedule.getAll(buf);
      memory.saveRelaySchedule(buf);
    }
  #endif

  exitInput.read();

  if(exitInput.isPressed()) {
    relay.activateByExternalInput();
    exitLed.on();
  }
  else {
    if(exitInput.isReleased()) {
      exitLed.off();
      relay.deactivateByExternalInput();
    }
  }
//   #if(DEBUG == true)
//     printFreeRam();
//   #endif 
}
