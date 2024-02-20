#include "Arduino.h"
#include "config.h"
#include "SPIFFS.h"
#include "schedule.h"


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
            Serial.println("[SPIFFS] Formatting failed. Restarting ESP in 3 seconds");
          #endif
          delay(3000);
          ESP.restart();
        }
      }
      #if(DEBUG == true && DEBUG_MEMORY == true)
        Serial.println("[SPIFFS] Configured");
      #endif
//      File root = SPIFFS.open("/");
//      File file = root.openNextFile();
//     
//      while(file){
//     
//          Serial.print("FILE: ");
//          Serial.println(file.name());
//     
//          file = root.openNextFile();
//      }
    }
    
    void loadUsers() {
      File file = SPIFFS.open(userFilename);
      if(file) {
        if(file.size() > 10)  {
          DeserializationError error = deserializeJson(usersInfo, file);
          if(error) {
            #if(DEBUG == true && DEBUG_MEMORY == true)
              Serial.print("[SPIFFS] Failed to load users info: ");
              Serial.println(error.f_str());
            #endif
            usersInfo.clear();
          }
          else {
            #if(DEBUG == true && DEBUG_MEMORY == true)
              Serial.println("[SPIFFS] Users loaded");
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
          Serial.println("[SPIFFS] Users updated");
        #endif
      }
      else {
        #if(DEBUG == true && DEBUG_MEMORY == true)
          Serial.println("[SPIFFS] Users failed to updated");
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
            setDefaultsSettings();
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
      systemInfo["t"] = LOCK_TYPE;
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
      systemInfo["t"]  = LOCK_TYPE;
      updateSystemInfo();
    }
    
    void resetProgramAccess() {
      #if(DEBUG == true && DEBUG_MEMORY == true)
        Serial.println("[SPIFFS] Resettings default Program access code.");
      #endif
      systemInfo["ac"] = PROG_PASSWORD;
      updateSystemInfo();
    }


    void saveRelaySchedule(char * buffer) {
      File file = SPIFFS.open("/relayFunctions.json", "w");
      if(file) {
        file.print(buffer);
        file.close();
        #if(DEBUG == true && DEBUG_MEMORY == true)
          Serial.printf("[SPIFFS] Relay schedule saved %s\n", buffer);
        #endif
      }
    }

    void loadRealySchedule(ScheduleManager *schedule) {
      #if(DEBUG == true && DEBUG_MEMORY == true)
        Serial.println("[SPIFFS] Loading relay schedule.");
      #endif
      File file = SPIFFS.open("/relayFunctions.json");
      if(file) {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, file);
        if(error) {
          #if(DEBUG == true && DEBUG_MEMORY == true)
            Serial.print("[SPIFFS] Error parsing relay schedule: ");
            Serial.println(error.f_str());
          #endif
          return;
        }
        int len = doc.size();
        for (uint8_t i=0; i<len; i++) {
          const uint8_t index = doc[i]["id"];
          const char    *n = doc[i]["n"];
          unsigned long sd = doc[i]["sd"];
          unsigned long ed = doc[i]["ed"];
          unsigned long st = doc[i]["st"];
          unsigned long et = doc[i]["et"];
          uint8_t wd       = doc[i]["wd"];
          uint8_t sf       = doc[i]["sf"];
          uint8_t ef       = doc[i]["ef"];
          schedule->addAtSpecificIndex(index, n, sd, ed, st, et, wd, sf, ef);
        }
        doc.clear();
      }
    }

    void loadHistory() {
      #if(DEBUG == true && DEBUG_MEMORY == true)
        Serial.println("[SPIFFS] Loading history.");
      #endif

      File file = SPIFFS.open("/history.json");
      if(file) {
        DeserializationError error = deserializeJson(history, file);
        if(error) {
          #if(DEBUG == true && DEBUG_MEMORY == true)
            Serial.print("[SPIFFS] Error parsing history data: ");
            Serial.println(error.f_str());
          #endif
          history.clear();
          return;
        }
        #if(DEBUG == true && DEBUG_MEMORY == true)
          Serial.println("[SPIFFS] History loaded.");
        #endif
        return;
      }
      #if(DEBUG == true && DEBUG_MEMORY == true)
        Serial.println("[SPIFFS] Error Loading history.");
      #endif
    }

    void updateHistory() {
      File file = SPIFFS.open("/history.json", FILE_WRITE);
      if(file) {
        serializeJson(history, file);
        file.close();
        #if(DEBUG == true && DEBUG_MEMORY == true)
          Serial.println("[SPIFFS] History updated.");
        #endif
        return;
      }
      #if(DEBUG == true && DEBUG_MEMORY == true)
        Serial.println("[SPIFFS] Failed to updat history.");
      #endif
    }
};
