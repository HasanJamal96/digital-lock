#define DEBUG     		true
#define DEBUG_MEMORY  true
#define DEBUG_SERVER  true


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