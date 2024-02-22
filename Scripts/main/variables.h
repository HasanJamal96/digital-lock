#ifndef VARIABLES_H
#define VARIABLES_H

#include "config.h"

typedef enum : uint8_t {
  NORMAL,
  WAIT_PROGRAMING_CODE,
  PROGRAMING,
  RESET_PASSWORD,
  RESET_ALL,
} device_state_t;


typedef enum : uint8_t {
  NONE,
  ADD_EDIT_DELETE,
  ADD,
  EDIT,
  DELETE,
  SELECT_FUNCTIONS,
  NEW_PASSWORD,
} programing_step_t;



bool addUser           = false;
bool editUser          = false;
bool deleteUser        = false;
bool userConfirmed     = false;
bool functionConfirmed = false;
uint8_t selectedFunction = 0;
int userId = -1;

/*
  Total Pins
    Input/Output = 15, 2, 4, 16, 17, 18, 19, 21, 22, 23, 13, 12, 14, 27, 26, 33, 32
    Input only   = 34, 35, 36, 39
  
  Extra Pins
    15, 16, 17

  SPI Pins - Ethernet shield
    5, 18, 19, 23
  
  I2C Pins
    21, 22
*/
const uint8_t RELAY_PIN		     = 27;
const uint8_t RELAY_LED_PIN    = 26;
const uint8_t BUZZER_PIN       = 4;
const uint8_t RESET_BTN_PIN    = 32;
const uint8_t EXIT_LED_PIN     = 25;
const uint8_t EXIT_INPUT_PIN   = 2;
const uint8_t KEYPAD_LED_PIN   = 33;


// keypad

const uint8_t ROWS = 4;
const uint8_t COLS = 3;

byte KEYPAD_R_PINS[ROWS] = {36, 39, 34, 35}; // R1, R2, R3, R4
byte KEYPAD_C_PINS[COLS] = {14, 12, 13};     // C1, C2, C3

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};


bool           backLightState = false;
const uint8_t  BACKLIGHT_LED_LIN = 27;
const uint16_t BACKLIGHT_OFF_AFTER = 30000; // time in milli seconds

const uint8_t MAX_HISTORY = 10;

bool enterState  = false;
bool cancelState = false;
unsigned long enterPressedAt  = 0;
unsigned long cancelPressedAt = 0;


const uint16_t EXIT_ENTER_PROGRAM_MODE = 5000; 


char    passcode[MAX_CODE_LENGTH+1] = {};
uint8_t passcodeIndex = 0;
const uint16_t KEY_RESET_DELAY = 2000;
unsigned long lastKeyPressed = 0;


unsigned long deviceStateChangeAt = 0;
uint8_t consectiveInvalidEntries = 0;
uint32_t lockOutStartTime = 0;



// System
char deviceName[20];
char apName[20];
char apPass[20];

bool addByServer    = false;
bool editByServer   = false;
bool deleteByServer = false;
bool systemByServer = false;

// User codes
uint16_t registeredUsersCount = 0;
uint8_t maxPasscodeLength     = CODE_LENGTH;
uint8_t currentPassLength     = 4;



// Programing codes
const uint8_t PROGRAM_ACCESS_CODE_LEN = 6;
char programAccessCode[PROGRAM_ACCESS_CODE_LEN+1] = {};


bool sleepMode = false;
bool lockOut   = false;
bool lockTheDevice = false;

bool isClientConnected = false;

#if (LOCK_TYPE > 1)
typedef enum : uint8_t {
  INTERNET_CONNECTED,
  INTERNET_DISCONNECTED,
  INTERNET_CONNECTING,
}internet_status_t;

typedef enum : uint8_t {
  ERROR_NONE,
  ERROR_NO_INTERNET,
  ERROR_NO_WIFI,
  ERROR_INVALID_PASSWORD,
  ERROR_NO_CABLE_CONNECTED,
}internet_errors_t;

char wifiName[30];
char wifiPass[30];

bool forceReconnect = true;

unsigned long connectionStartTime = 0;
const uint16_t  RETRY_AFTER = 20000; // time in milli seconds

char scanResult[200];
char deviceIdForServer[20];

bool wifiConnected  = false;
bool wifiConnecting = false;
bool ethernetConnected = false;
char enternetConncetionError[50];
char wifiConncetionError[50];

bool isWebsocketConnected = false;

#endif

#endif // VARIABLES_H
