/*
  0 -> DL100
  1 -> DL1500 Timer
  2 -> DL1500 Wi-Fi
  3 -> DL2000
*/
#define LOCK_TYPE 1

#define DEBUG     		true // false will disable all debug logs
#define DEBUG_RTC     true // false will only disable RTC related logs 
#define DEBUG_MEMORY  true // false will only disable Memory related logs
#define DEBUG_SERVER  true // false will only disable Server related logs


#if (DEBUG == true)
#define BAUDRATE	115200
#endif


// Relay Modes
// NONE      = 0
// MOMENTARY = 1
// LATCH     = 2
// UNLATCH   = 3
// TOGGLE    = 4


// Default values

#define DEVICE_NAME       "DL100"     // should be less than 20 characters
#define AP_SSID           "DL100"     // should be less than 20 characters
#define AP_PASS           "password"  // should be less than 20 characters

#define PROG_PASSWORD     "000000"    // should be equal to 6 characters
#define RELAY_FUNCTION    1           // as shown above
#define MOMENTARY_DELAY   2000        // time in milli seconds
#define LIMIT_USE         0           // 0-> Off, 1->On
#define USER_NAME         ""          // should be less than 20 characters
#define CODE_LENGTH       4
#define MAX_CODE_LENGTH   6

#define SLEEP_MODE        0           // 0-> Off, 1->On
#define LOCKOUT           0           // 0-> Off, 1->On

#define MAX_INVALID_ATTEMPTS  5

#define MODE_RESET_BACK_TO_NORMAL_AFTER  240000 // time in milli seconds


#if (LOCK_TYPE == 0)
  #define MAX_SUPPORTED_USERS  100
#elif (LOCK_TYPE < 2)
  #define MAX_SUPPORTED_USERS  1000
#else
  #define MAX_SUPPORTED_USERS  10000
#endif
