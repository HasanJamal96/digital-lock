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
  SPI Pins - Ethernet shield
  5, 18, 19, 23
  
  I2C Pins
  21, 22
*/
const uint8_t RELAY_PIN		     = 27;
const uint8_t BUZZER_PIN       = 4;
const uint8_t RESET_BTN_PIN    = 32;
const uint8_t RELAY_LED_PIN    = 26;
const uint8_t EXIT_LED_PIN     = 25;
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


bool           backLightState = true;
const uint8_t  BACKLIGHT_LED_LIN = 27;
const uint16_t BACKLIGHT_OFF_AFTER = 30000; // time in milli seconds


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



