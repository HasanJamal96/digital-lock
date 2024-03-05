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


const char *JSON_PARSE_ERROR   = "{\"success\":0,\"error\":\"unable to parse json\"}";
const char *successMsg = "{\"success\":1}";

void routes();
uint8_t updatesByServer();
void relayFunctionsForServer(uint8_t func); // function's working defined in main.ino


void serverLoop() {
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



#if(DEBUG == true && DEBUG_SERVER == true)
void apConnectionCallback(WiFiEvent_t event) {
  if(event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
      Serial.printf("[Internet] AP client connected\n");
    isClientConnected = true;
  }
  else if(event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
    // #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Internet] AP client disconnected\n");
    // #endif
    isClientConnected = false;
  }
}
#endif


void initServer() {
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Initializing");
  #endif
  
  // WiFi.mode(WIFI_MODE_APSTA);
  WiFi.mode(WIFI_MODE_AP);
  // WiFi.eraseAP();
  // WiFi.begin("EBMACS-2.4GHz", "ebmacs1234567890");
  #if(DEBUG == true && DEBUG_SERVER == true)
    WiFi.onEvent(apConnectionCallback);
  #endif

  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Initialized");
  #endif
}


void routes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", HTML);
  });
  server.on("/assets/index-0379c66a.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/css", CSS);
  });
  server.on("/assets/index-90cf9535.js", HTTP_GET, [](AsyncWebServerRequest *request) {
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
    systemInfo["t"]  = LOCK_TYPE;
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
      Serial.println(error.f_str());
      request->send(200, "application/json", JSON_PARSE_ERROR);
      return;
    }
    addByServer = true;
    if(updatesByServer())
      request->send(200, "application/json", successMsg);
    else
      request->send(200, "application/json", "{\"success\":0,\"error\":\"Unable to add new user\"}");
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
      request->send(200, "application/json", "{\"success\":0,\"error\":\"User not found\"}");
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
    uint8_t x = updatesByServer();
    if(x == 1)
      request->send(200, "application/json", successMsg);
    else if(x == 2)
      request->send(200, "application/json", "{\"success\":0,\"error\":\"New password assosiate with other user\"}");
    else
      request->send(200, "application/json", "{\"success\":0,\"error\":\"User not found\"}");
  });
  server.on("/change-state",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DynamicJsonDocument doc(50);
    DeserializationError error = deserializeJson(doc, rec);
    if(error) {
      request->send(200, "application/json", JSON_PARSE_ERROR);
      return;
    }
    const int x = doc["s"];
    if(x <= R_CLOSE) {
      if(relay.setState(x)) {
        request->send(200, "application/json", successMsg);
      }
      else {
        request->send(200, "application/json", "{\"success\":0,\"error\":\"Invalid state\"}");
      }
    }
    else {
      request->send(200, "application/json", "{\"success\":0,\"error\":\"Invalid state\"}");
    }
  });
  server.on("/get-state", HTTP_GET, [](AsyncWebServerRequest *request) {
    char msg[50];
    strcpy(msg, "{\"success\":1,\"s\":\"");
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
  server.on("/get-history", HTTP_GET, [](AsyncWebServerRequest *request) {
    #if (DEBUG == true && DEBUG_SERVER == true)
      Serial.println("[Server] Sending history");
    #endif
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(history, *response);
    request->send(response);
  });
#if (LOCK_TYPE > 0)
    server.on("/set-time",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DynamicJsonDocument doc(100);
    DeserializationError error = deserializeJson(doc, rec);
    if(error) {
      request->send(200, "application/json", JSON_PARSE_ERROR);
      return;
    }
    const uint64_t unixTime = doc["millis"];
    const float tz = doc["tz"];
    uint32_t seconds = tz * 3600;
    uint32_t timeToSet = (unixTime/1000) - (seconds);
    if(rtc.setRtcTime(timeToSet, tz))
      request->send(200, "application/json", successMsg);
    else
      request->send(200, "application/json", "{\"success\":0,\"error\":\"RTC not working\"}");
    });
    server.on("/get-time", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(rtc.getRTCState()) {
        float tz = rtc.getTZ();

        DynamicJsonDocument doc(50);
        doc["m"] = now() + (3600 * tz);
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        serializeJson(doc, *response);
        request->send(response);
      }
      else {
        request->send(200, "application/json", "{\"success\":0,\"error\":\"RTC not working\"}");
      }
    });
    server.on("/get-relay-schedules", HTTP_GET, [](AsyncWebServerRequest *request) {
      char buffer[1200];
      schedule.getAll(buffer);
      request->send(200, "application/json", buffer);
    });
    server.on("/add-relay-schedule",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      String rec = "";
      for(int i=0; i<len; i++)
        rec += (char)data[i];
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, rec);
      if(error) {
        request->send(200, "application/json", JSON_PARSE_ERROR);
        return;
      }
      const char    *n = doc["n"];
      unsigned long sd = doc["sd"];
      unsigned long ed = doc["ed"];
      unsigned long st = doc["st"];
      unsigned long et = doc["et"];
      uint8_t wd       = doc["wd"];
      uint8_t sf = doc["sf"];
      uint8_t ef = doc["ef"];

      sd = sd + 61200;
      ed = ed + 61200;
      if(schedule.add(n, sd, ed, st, et, wd, sf, ef))
        request->send(200, "application/json", successMsg);
      else
        request->send(200, "application/json", "{\"success\":0,\"error\":\"Not enough space\"}");
    });
    server.on("/delete-relay-schedule",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      String rec = "";
      for(int i=0; i<len; i++)
        rec += (char)data[i];
      DynamicJsonDocument doc(50);
      DeserializationError error = deserializeJson(doc, rec);
      if(error) {
        request->send(200, "application/json", JSON_PARSE_ERROR);
        return;
      }
      uint8_t id = doc["p"];
      if(schedule.remove(id))
        request->send(200, "application/json", successMsg);
      else
        request->send(200, "application/json", "{\"success\":0,\"error\":\"Invalid id\"}");
    });

    server.on("/update-relay-schedule",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      String rec = "";
      for(int i=0; i<len; i++)
        rec += (char)data[i];
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, rec);
      if(error) {
        request->send(200, "application/json", JSON_PARSE_ERROR);
        return;
      }
      uint8_t       id = doc["i"];
      const char    *n = doc["n"];
      unsigned long sd = doc["sd"];
      unsigned long ed = doc["ed"];
      unsigned long st = doc["st"];
      unsigned long et = doc["et"];
      uint8_t       wd = doc["wd"];
      unsigned long sf = doc["sf"];
      unsigned long ef = doc["ef"];
      sd = sd + 61200;
      ed = ed + 61200;
      if(schedule.update(id, n, sd, ed, st, et, wd, sf, ef))
        request->send(200, "application/json", successMsg);
      else
        request->send(200, "application/json", "{\"success\":0,\"error\":\"Invalid id\"}");
    });
#endif
  
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });
}

void startServer() {
  WiFi.softAPdisconnect();
  delay(500);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, apPass, 1, 0, 1);
  delay(500);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("content-type"));
  server.begin();
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.printf("[Server] Started with SSID: %s, and password %s\n", apName, apPass);
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
