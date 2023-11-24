#include "config.h"
#include <WiFi.h>
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
      Serial.printf("[Internet] AP client connected\n");
    #endif
    isClientConnected = true;
  }
  else if(event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Internet] AP client disconnected\n");
    #endif
    isClientConnected = false;
  }
  else if(event == ARDUINO_EVENT_WIFI_SCAN_DONE) {
    strcpy(scanResult, "[");
    int n = WiFi.scanComplete();
    if(n && n != -2) {
      for (int i=0; i<n; ++i)
        strcpy(scanResult, ("\"" + WiFi.SSID(i) + "\"").c_str());
        if(i>0 && i != (n-1)) {
          strcpy(scanResult, ",");
        }
      WiFi.scanDelete();
    }
    strcpy(scanResult, "]");
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Scan complete %s\n", scanResult);
    #endif
  }
}

void internetDisconnectCallback(WiFiEvent_t event, WiFiEventInfo_t info) {
  uint8_t res = info.disconnected.reason;
  if(res == 15 || res == 202) {
    Serial.printf("[Advance Server] Authentication failed - Invalid password\n");
    strcpy(wifiConncetionError, "Authentication failed - Invalid password");
  }
  else if(res == 201) {
    Serial.printf("[Advance Server] WiFi is not in range\n");
    strcpy(wifiConncetionError, "WiFi is not in range");
  }
  else if(res == 3) {
    Serial.printf("[Advance Server] WiFi is not answering\n");
    strcpy(wifiConncetionError, "WiFi is not answering");
  }
  else {
    strcpy(wifiConncetionError, "Unknown reason");
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Unknown reason\n");
    #endif
  }
  #if(DEBUG == true && DEBUG_SERVER == true)
    Serial.printf("[Advance Server] Disconnected\n");
  #endif
  wifiConnected = false;
}



void connectWiFi() {
  if(len(wifiName) > 0) {
    #if (DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Connecting to WiFi %s\n", wifiName);
    #endif
    WiFI.begin(wifiName, wifiPass);
  }
  connectionStartTime = millis();
}



void serverLoop() {
  if(!wifiConnected) {
    if(internetStatus == INTERNET_CONNECTING) {
      if(millis() - connectionStartTime >= RETRY_AFTER) {
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
  WiFi.onEvent(internetDisconnectCallback, SYSTEM_EVENT_STA_DISCONNECTED);
  
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
      request->send(SPIFFS, "/index.html");
  }
};


void routes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", HTML);
  });

  server.on("/jquery.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/javascript", JS);
  });
  server.on("/connect_wifi",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec;
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    request->send(200, "text/plain", "ok");
    uint8_t index1 = rec.indexOf(',', 0);
    Serial.println(rec.substring(0,index1));
    Serial.println(rec.substring(index1+1,len));
  });

  server.on("/scan_wifi",HTTP_GET,[](AsyncWebServerRequest * request){
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Scanning\n");
    #endif
    request->send(200, "text/plain", "ok");
    WiFi.scanNetworks(true);
  });


  server.on("/get_scan_result",HTTP_GET,[](AsyncWebServerRequest * request){
    request->send(200, "text/plain", scanResult);
  });
  
  server.on("/connection_status",HTTP_GET,[](AsyncWebServerRequest * request) {
    char InternetConnectionStatus[100];
    strcpy(InternetConnectionStatus, "{\"wifi\":\"");
    if(wifiConnected) {
      strcpy(InternetConnectionStatus, "1\"");
    }
    else {
      strcpy(InternetConnectionStatus, "0\",\"e\":\"");
      strcpy(InternetConnectionStatus, wifiConncetionError);
    }
    strcpy(InternetConnectionStatus, ",\"eth\":\"");
    if(ethernetConnected) {
      strcpy(InternetConnectionStatus, "1\"");
    }
    else {
      strcpy(InternetConnectionStatus, "0\",\"e\":\"");
      strcpy(InternetConnectionStatus, enternetConncetionError);
    }
      
    request->send(200, "text/plain", InternetConnectionStatus);
  }
}


void startServer() {
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("content-type"));
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