#include "ArduinoJson/Json/JsonSerializer.hpp"
#include "config.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include "basic_webpages.h"
#include <ESPAsyncWebServer.h>

IPAddress apIP(8,8,8,8);
DNSServer dnsServer;
AsyncWebServer server(80);


const char *JSON_PARSE_ERROR   = "{\"success\":\"0\",\"error\":\"unable to parse json\"}";
const char *successMsg = "{\"success\":\"1\"}";

void routes();
bool updatesByServer();
void relayFunctionsForServer(uint8_t func); // function's working defined in main.ino



void serverLoop() {
  if(isClientConnected)
    dnsServer.processNextRequest();
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
      request->redirect("http://8.8.8.8/");
    }
//    else
//      request->send(SPIFFS, "/index.html");
  }
};



void apConnectionCallback(WiFiEvent_t event) {
  if(event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
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
}


void initServer() {
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Initializing");
  #endif
  
  WiFi.mode(WIFI_MODE_APSTA);
  // WiFi.begin("EBMACS-2.4GHz", "ebmacs1234567890");
  // delay(6000);
  // Serial.println(WiFi.localIP());
  WiFi.onEvent(apConnectionCallback);
  
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Initialized");
  #endif
}


void routes() {
//  server.on("/assets/<filename>", HTTP_GET, [](AsyncWebServerRequest *request) {
//    request->send(SPIFFS, request->url(), "text/javascript");
//  });
//  
//  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", HTML);
  });
  server.on("/assets/index-7edf4032.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/css", CSS);
  });
  server.on("/assets/index-d58f8346.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/javascript", JS);
  });
  server.on("/digi-lock-ico.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "image/svg+xml", ICON);
  });
  
  server.on("/get-users", HTTP_GET, [](AsyncWebServerRequest *request) {
    #if (DEBUG == true && DEBUG_SERVER == true)
      Serial.println("[Server] Sending users ");
    #endif
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(usersInfo, *response);
    request->send(response);
  });
  server.on("/get-system-info", HTTP_GET, [](AsyncWebServerRequest *request) {
    #if (DEBUG == true && DEBUG_SERVER == true)
      Serial.println("[Server] Sending system info ");
    #endif
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(systemInfo, *response);
    request->send(response);
  });
  
  server.on("/update-system-info",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(systemInfo, rec);
    if(error) {
      request->send(200, "application/json", JSON_PARSE_ERROR);
      return;
    }
    systemByServer = true;
    if(updatesByServer())
      request->send(200, "application/json", successMsg);
    else
      request->send(200, "application/json", JSON_PARSE_ERROR);
  });
  server.on("/new-user",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(newUser, rec);
    if(error) {
      request->send(200, "application/json", JSON_PARSE_ERROR);
      return;
    }
    addByServer = true;
    if(updatesByServer())
      request->send(200, "application/json", successMsg);
    else
      request->send(200, "application/json", "{\"success\":\"0\",\"error\":\"unable to add new user\"}");
  });
  server.on("/delete-user",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(newUser, rec);
    if(error) {
      request->send(200, "application/json", JSON_PARSE_ERROR);
      return;
    }
    deleteByServer = true;
    if(updatesByServer())
      request->send(200, "application/json", successMsg);
    else
      request->send(200, "application/json", "{\"success\":\"0\",\"error\":\"user not found\"}");
  });
  server.on("/update-user",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(newUser, rec);
    if(error) {
      request->send(200, "application/json", JSON_PARSE_ERROR);
      return;
    }
    editByServer = true;
    if(updatesByServer())
      request->send(200, "application/json", successMsg);
    else
      request->send(200, "application/json", "{\"success\":\"0\",\"error\":\"user not found\"}");
  });
  server.on("/change-state",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DynamicJsonDocument serverData(50);
    DeserializationError error = deserializeJson(serverData, rec);
    if(error) {
      request->send(200, "application/json", JSON_PARSE_ERROR);
      return;
    }
    const int x = serverData["s"];
    if(x < 2) {
      switch(x) {
        case 0: // off
          relay.off();
          break;
        case 1: // on
          relay.on();
          break;
      }
      request->send(200, "application/json", successMsg);
    }
    else {
      request->send(200, "application/json", "{\"success\":\"0\",\"error\":\"invalid state\"}");
    }
  });
  #if (LOCK_TYPE > 0)
    server.on("/set-time",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DynamicJsonDocument serverData(50);
    DeserializationError error = deserializeJson(serverData, rec);
    if(error) {
      request->send(200, "application/json", JSON_PARSE_ERROR);
      return;
    }
    const uint64_t unixTime = serverData["millis"];
    
    if(rtc.setRtcTime(unixTime/1000))
      request->send(200, "application/json", successMsg);
    else
      request->send(200, "application/json", "{\"success\":\"0\",\"error\":\"RTC not working\"}");
    });
  #endif
  server.on("/get-state", HTTP_GET, [](AsyncWebServerRequest *request) {
    char msg[50];
    strcpy(msg, "{\"success\":\"1\",\"s\":\"");
    if(relay.status()) {
      strcat(msg, "1\",\"e\":\"");
    }
    else {
      strcat(msg, "0\",\"e\":\"");
    }
    if(exitInput.status()) {
      strcat(msg, "1\"}");
    }
    else {
      strcat(msg, "0\"}");
    }
    request->send(200, "application/json", msg);
    

  });
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });
}

void startServer() {
//  WiFi.disconnect();
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, apPass);
  delay(1000);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("content-type"));
  server.begin();
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Started");
  #endif
}

void closeServer() {
  server.end();
  server.reset();
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Closed");
  #endif
}


void restartServer() {
  closeServer();
  startServer();
}
