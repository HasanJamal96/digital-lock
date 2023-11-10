#include "Arduino.h"
#include "config.h"
#include "SPIFFS.h"


const char *userFilename = "/users.json";
const char *systemFilename = "/system.json";


class Memory {
  public:
    void init() {
      if(!SPIFFS.begin()) {
        #if(DEBUG == true && DEBUG_MEMORY == true)
          Serial.println("[SPIFFS] Failed to init, Formatting memory...");
        #endif
        if(SPIFFS.format()) {
          #if(DEBUG == true && DEBUG_MEMORY == true)
            Serial.println("[SPIFFS] Formated successfully");
          #endif
        }
        else {
          #if(DEBUG == true && DEBUG_MEMORY == true)
            Serial.println("[SPIFFS] Formatting failed. Restarting ESP in 3 seconds.");
          #endif
          delay(3000);
          ESP.restart();
        }
      }
      #if(DEBUG == true && DEBUG_MEMORY == true)
        Serial.println("[SPIFFS] Configured");
      #endif
    }
    
    void loadUsers() {
      File file = SPIFFS.open(userFilename);
      if(file) {
        if(file.size() > 10)  {
          DeserializationError error = deserializeJson(usersInfo, file);
          if(error) {
            #if(DEBUG == true && DEBUG_MEMORY == true)
              Serial.println("[SPIFFS] Failed to load users info.");
            #endif
          }
          else {
            #if(DEBUG == true && DEBUG_MEMORY == true)
              Serial.println("[SPIFFS] Users loaded.");
            #endif
          }
        }
        file.close();
      }
      else {
        usersInfo.clear();
        updateUsers();
      }
    }
    
    void updateUsers() {
      File file = SPIFFS.open(userFilename, FILE_WRITE);
      if(file) {
        serializeJson(usersInfo, file);
        file.close();
        #if(DEBUG == true && DEBUG_MEMORY == true)
          Serial.println("[SPIFFS] Users updated.");
        #endif
      }
      else {
        #if(DEBUG == true && DEBUG_MEMORY == true)
          Serial.println("[SPIFFS] Users failed to updated.");
        #endif
      }
    }
    
    void resetUsers() {
      usersInfo.clear();
      registeredUsersCount = 0;
      updateUsers();
    }
    
    
    void loadSystemInfo() {
      File file = SPIFFS.open(systemFilename);
      if(file) {
        if(file.size() > 10) {
          DeserializationError error = deserializeJson(systemInfo, file);
          if(error) {
            #if(DEBUG == true && DEBUG_MEMORY == true)
              Serial.println("[SPIFFS] Failed to load system information.");
            #endif
          }
        }
        else {
          #if(DEBUG == true && DEBUG_MEMORY == true)
            Serial.println("[SPIFFS] System information file is empty.");
          #endif
          setDefaultsSettings();
        }
        file.close();
      }
      else {
        #if(DEBUG == true && DEBUG_MEMORY == true)
          Serial.println("[SPIFFS] Failed to open file system information.");
        #endif
        setDefaultsSettings();
      }
    }
    
    
    void updateSystemInfo() {
      File file = SPIFFS.open(systemFilename, FILE_WRITE);
      serializeJson(systemInfo, file);
    }
    
    void setDefaultsSettings() {
      #if(DEBUG == true && DEBUG_MEMORY == true)
        Serial.println("[SPIFFS] Resettings default settings.");
      #endif
      systemInfo.clear();
      systemInfo["dn"] = DEVICE_NAME;
      systemInfo["wn"] = AP_SSID;
      systemInfo["wp"] = AP_PASS;
      systemInfo["cl"] = CODE_LENGTH;
      systemInfo["lo"] = LOCKOUT;
      systemInfo["sm"] = SLEEP_MODE;
      systemInfo["ac"] = PROG_PASSWORD;
      updateSystemInfo();
    }
    
    void resetProgramAccess() {
      #if(DEBUG == true && DEBUG_MEMORY == true)
        Serial.println("[SPIFFS] Resettings default Program access code.");
      #endif
      systemInfo["ac"] = PROG_PASSWORD;
      updateSystemInfo();
    }
};