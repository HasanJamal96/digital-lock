#include "config.h"
#include <WiFi.h>



internet_status_t internetStatus = INTERNET_DISCONNECTED;
internet_errors_t internetErrors = ERROR_NONE;


void connectWiFi() {
  if(!enableEthernet) {
    #if (DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Connecting to WiFi %s\n", wifiName);
    #endif
    WiFI.begin(wifiName, wifiPass);
  }
  else {
    
  }
  connectionStartTime = millis();
  
}



void serverLoop() {
  if(internetStatus == INTERNET_CONNECTING) {
    if(millis() - connectionStartTime >= RETRY_AFTER) {
      connectWiFi();
    }
  }
}


void initServer() {
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Initializing");
  #endif
  
  WiFi.mode(WIFI_MODE_APSTA);
  
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Initialized");
  #endif
}



void startServer() {
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Start");
  #endif
}

void closeServer() {
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Closed");
  #endif
}


void restartServer() {
  closeServer();
  startServer();
}