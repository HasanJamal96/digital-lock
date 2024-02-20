#ifndef config_h
#define config_h

/*
  0 -> DL100
  1 -> DL1500 Timer
  2 -> DL1500 Wi-Fi
  3 -> DL2000
*/
#define LOCK_TYPE 1

#define DEBUG     		true  // false will disable all debug logs
#define DEBUG_RTC     true  // false will only disable RTC related logs
#define DEBUG_MEMORY  true  // false will only disable Memory related logs
#define DEBUG_SERVER  true  // false will only disable Server related logs


#if (DEBUG == true)
#define BAUDRATE	115200
#endif


// Relay Modes
// NONE      = 0
// MOMENTARY = 1
// LATCH     = 2
// UNLATCH   = 3
// TOGGLE    = 4
// SCHEDULED = 5

const uint8_t R_NONE       = 0;
const uint8_t R_MOMENTARY  = 1;
const uint8_t R_LATCH      = 2;
const uint8_t R_UNLATCH    = 3;
const uint8_t R_TOGGLE     = 4;
const uint8_t R_SCHEDULED  = 5;
const uint8_t R_HOLD       = 6;
const uint8_t R_OPEN       = 7;
const uint8_t R_CLOSE      = 8;

// Default values

#define DEVICE_NAME       "DL100"     // should be less than 20 characters
#define AP_SSID           "DL100"     // should be less than 20 characters
#define AP_PASS           "password"  // should be less than 20 characters

#define PROG_PASSWORD     "000000"    // should be equal to 6 characters
#define RELAY_FUNCTION    1           // as shown above
#define MOMENTARY_DELAY   2           // time in seconds
#define LIMIT_USE         0           // 0-> Off, 1->On
#define USER_NAME         ""          // should be less than 20 characters
#define CODE_LENGTH       4
#define MAX_CODE_LENGTH   6

#define SLEEP_MODE        0           // 0-> Off, 1->On
#define LOCKOUT           0           // 0-> Off, 1->On

#define MAX_INVALID_ATTEMPTS  5
#define LOCK_WAIT_TIME        30000 //  time in milli seconds

#define MODE_RESET_BACK_TO_NORMAL_AFTER  240000 // time in milli seconds


#if (LOCK_TYPE == 0)
  #define MAX_SUPPORTED_USERS  100
#elif (LOCK_TYPE == 1)
  #define MAX_SUPPORTED_USERS  1000
#else
  #define MAX_SUPPORTED_USERS  10000
#endif
#endif



#if (LOCK_TYPE > 0)
#define MAGIC_NUMBER  43
#define MAGIC_NUMBER_LOCATION      1
#define RELAY_STATUS_LOCATION      2
#define ACTIVE_SCHEDULED_LOCATION  3
#define INVALID_SCHEDULE          255
#endif
