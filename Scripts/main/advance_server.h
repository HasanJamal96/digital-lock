#include "config.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include "webpages.h"
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

IPAddress apIP(8,8,8,8);
DNSServer dnsServer;
AsyncWebServer server(80);


internet_status_t internetStatus = INTERNET_DISCONNECTED;
internet_errors_t internetErrors = ERROR_NONE;

void routes();
void startServer();


void internetCallback(WiFiEvent_t event) {
  if(event == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Connected\n"); 
    #endif
  }
  else if(event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Got IP address: ");
       Serial.println(WiFi.localIP());
    #endif
    wifiConnected = true;
  }
  else if(event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] AP client connected\n");
    #endif
    isClientConnected = true;
  }
  else if(event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] AP client disconnected\n");
    #endif
    isClientConnected = false;
  }
  else if(event == ARDUINO_EVENT_WIFI_SCAN_DONE) {
    strcpy(scanResult, "[");
    int n = WiFi.scanComplete();
    if(n && n != -2) {
      for (int i=0; i<n; ++i) {
        strcat(scanResult, ("\"" + WiFi.SSID(i) + "\",").c_str());
      }
      WiFi.scanDelete();
    }
    scanResult[strlen(scanResult)-1] = ']';
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Scan complete: %s\n", scanResult);
    #endif
  }
}

// void internetDisconnectCallback(WiFiEvent_t event, wifi_err_reason_t info) {
//   uint8_t res = info
//   if(res == 15 || res == 202) {
//     Serial.printf("[Advance Server] Authentication failed - Invalid password\n");
//     strcpy(wifiConncetionError, "Authentication failed - Invalid password");
//   }
//   else if(res == 201) {
//     Serial.printf("[Advance Server] WiFi is not in range\n");
//     strcpy(wifiConncetionError, "WiFi is not in range");
//   }
//   else if(res == 3) {
//     Serial.printf("[Advance Server] WiFi is not answering\n");
//     strcpy(wifiConncetionError, "WiFi is not answering");
//   }
//   else {
//     strcpy(wifiConncetionError, "Unknown reason");
//     #if(DEBUG == true && DEBUG_SERVER == true)
//       Serial.printf("[Advance Server] Unknown reason\n");
//     #endif
//   }
//   #if(DEBUG == true && DEBUG_SERVER == true)
//     Serial.printf("[Advance Server] Disconnected\n");
//   #endif
//   wifiConnected = false;
// }


void connectWiFi() {
  if(strlen(wifiName) > 0) {
    #if (DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Connecting to WiFi %s\n", wifiName);
    #endif
    WiFi.begin(wifiName, wifiPass);
  }
  connectionStartTime = millis();
}



void serverLoop() {
  if(!wifiConnected) {
    if(internetStatus != INTERNET_CONNECTED || forceReconnect) {
      if(millis() - connectionStartTime >= RETRY_AFTER) {
        forceReconnect = false;
        connectWiFi();
      }
    }
  }
  if(isClientConnected)
    dnsServer.processNextRequest();
}


void initServer() {
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Initializing");
  #endif
  
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.onEvent(internetCallback);
  // WiFi.onEvent(internetDisconnectCallback, SYSTEM_EVENT_STA_DISCONNECTED);

  
  String mac = WiFi.macAddress();
  mac.replace(":","");
  strcpy(deviceIdForServer, mac.c_str());
  
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Initialized");
  #endif
}


class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {
    routes();
  }
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request) {
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    if(request->url() == "/connecttest.txt" || request->url() == "/redirect" || request->url() == "/generate_204" || request->url() == "/fwlink" || request->url() == "/hotspot-detect.html") {
      Serial.println("Inside captive-portal");
      request->redirect("http://8.8.8.8/");
    }
    else
      request->send_P(200, "text/html", HTML);
  }
};


void routes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", HTML);
  });

  server.on("/jquery.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/javascript", JS);
  });
  server.on("/connect-wifi",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec;
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    request->send(200, "text/plain", "ok");
    uint8_t index1 = rec.indexOf(',', 0);

    strcpy(wifiName, (rec.substring(0,index1)).c_str());
    strcpy(wifiPass, rec.substring(index1+1,len).c_str());
    WiFi.disconnect();
    if(strlen(wifiName) > 0)
      forceReconnect = true;
    request->send(200, "text/plain", "Ok");
  });

  server.on("/scan-wifi",HTTP_GET,[](AsyncWebServerRequest * request){
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Scanning for nearby WiFi\n");
    #endif
    request->send(200, "text/plain", "Ok");
    WiFi.scanNetworks(true);
  });


  server.on("/scan-result",HTTP_GET,[](AsyncWebServerRequest * request){
    request->send(200, "text/plain", scanResult);
  });
  
  server.on("/connection-status",HTTP_GET,[](AsyncWebServerRequest * request) {
    char InternetConnectionStatus[100];
    strcpy(InternetConnectionStatus, "{\"wifi\":\"");
    if(wifiConnected) {
      strcat(InternetConnectionStatus, "1\"");
    }
    else {
      strcat(InternetConnectionStatus, "0\",\"e\":\"");
      strcat(InternetConnectionStatus, wifiConncetionError);
    }
    strcat(InternetConnectionStatus, ",\"eth\":\"");
    if(ethernetConnected) {
      strcat(InternetConnectionStatus, "1\"");
    }
    else {
      strcat(InternetConnectionStatus, "0\",\"e\":\"");
      strcat(InternetConnectionStatus, enternetConncetionError);
      strcat(InternetConnectionStatus, "\"}");
    }
    request->send(200, "text/plain", InternetConnectionStatus);
  });
}


void startServer() {
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, apPass);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("content-type"));
  server.begin();
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Started");
  #endif
}

void closeServer() {
  server.end();
  server.reset();
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Closed");
  #endif
}


void restartServer() {
  closeServer();
  startServer();
}