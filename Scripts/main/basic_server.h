#include "config.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

IPAddress apIP(8,8,8,8);
DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char *JSON_PARSE_ERROR   = "{\"success\":\"0\",\"error\":\"unable to parse json\"}";
const char *successMsg = "{\"success\":\"1\"}";

void routes();
bool updatesByServer();
void relayFunctionsForServer(uint8_t func);



void serverLoop() {
  dnsServer.processNextRequest();
  ws.cleanupClients();
}


/*
  open : {
          "f":"r", 
          "a":"o"
         }
  close: {
          "f":"r",
          "a":"c"
         }

  add user: {
              "f":"s",
              "a":"au",
              "d":"{user data}"
            }
  delete user: {
                 "f":"s",
                 "a":"du",
                 "p":"password"
               }
  edit user: {
                 "f":"s",
                 "a":"eu",
                 "p": "password",
                 "d": "{user data}"
               }
*/



////////////////////////////////////////////////////  Websocket

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT) {
   Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
   client->printf("Hello Client %u :)", client->id());
   client->ping();
  }
  else if(type == WS_EVT_DISCONNECT) {
   Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  }
  else if(type == WS_EVT_ERROR) {
   Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  }
  else if(type == WS_EVT_PONG) {
   Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  }
  else if(type == WS_EVT_DATA) {
    
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len) {
      char msgReceived[len];
      if(info->opcode == WS_TEXT) {
        for(size_t i=0; i < info->len; i++) {
          msgReceived[i] = (char) data[i];
        }
        DeserializationError error = deserializeJson(serverData, msgReceived);
        if(error) {
          client->text(JSON_PARSE_ERROR);
          return;
        }
        else {
          const char *x = serverData["f"];
          if(x[0] == 'r') {
            const char *a = serverData["a"];
            if(a[0] == 'o') {
              relayFunctionsForServer(0);
              client->text(successMsg);
            }
            else if(a[0] == 'c') {
              relayFunctionsForServer(1);
              client->text(successMsg);
            }
            else {
              client->text("{\"success\":\"0\",\"error\":\"invalid command\"}");
            }
          }
          else if(x[0] == 's') {
            const char *action = serverData["a"];
            char errorMsg[100];
            if(strcmp(action, "au") == 0) {
              addByServer = true;
              newUser = serverData["d"];
              strcpy(errorMsg, "{\"success\":\"0\",\"error\":\"unable to add new user\"}");
            }
            else if(strcmp(action, "du") == 0) {
              deleteByServer = true;
              newUser = serverData["d"];
              strcpy(errorMsg, "{\"success\":\"0\",\"error\":\"unable to add delete user\"}");
              newUser["p"] = serverData["p"];
            }
            else if(strcmp(action, "eu") == 0) {
              editByServer = true;
              newUser = serverData["d"];
              strcpy(errorMsg, "{\"success\":\"0\",\"error\":\"unable to add edit user\"}");
              newUser["p"] = serverData["p"];
            }
            else if(strcmp(action, "su") == 0) {
              systemByServer = true;
              systemInfo = serverData["d"];
              strcpy(errorMsg, "{\"success\":\"0\",\"error\":\"unable to add update system information\"}");
            }
            if(updatesByServer()) {
              client->text(successMsg);
            }
            else {
              client->text(errorMsg);
            }
          }
        }
      }
    }
  }
}







////////////////////////////////////////////////////  Webserver and Captive portal

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
    // else
    //   request->send(SPIFFS, "/index.html");
  }
};


void initServer() {
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Initializing");
  #endif
  
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.begin("EBMACS-2.4GHz", "ebmacs1234567890");
  
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Initialized");
  #endif
}


void routes() {
  server.on("/assets/<filename>", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println(request->url());
    request->send(SPIFFS, request->url(), "text/javascript");
  });
  
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
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
  
  server.on("/update-system-info",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(systemInfo, rec);
    if(error) {
      request->send(200, "text/plain", JSON_PARSE_ERROR);
      return;
    }
    systemByServer = true;
    if(updatesByServer())
      request->send(200, "text/plain", "{\"success\":\"1\"}");
    else
      request->send(200, "text/plain", JSON_PARSE_ERROR);
  });
  server.on("/new-user",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(newUser, rec);
    if(error) {
      request->send(200, "text/plain", JSON_PARSE_ERROR);
      return;
    }
    addByServer = true;
    Serial.println("/new-user");
    if(updatesByServer())
      request->send(200, "text/plain", "{\"success\":\"1\"}");
    else
      request->send(200, "text/plain", "{\"success\":\"0\",\"error\":\"unable to add new user\"}");
  });
  server.on("/delete-user",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(newUser, rec);
    if(error) {
      request->send(200, "text/plain", JSON_PARSE_ERROR);
      return;
    }
    deleteByServer = true;
    Serial.println("/delete-user");
    if(updatesByServer())
      request->send(200, "text/plain", "{\"success\":\"1\"}");
    else
      request->send(200, "text/plain", "{\"success\":\"0\",\"error\":\"user not found\"}");
  });
  server.on("/update-user",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(newUser, rec);
    if(error) {
      request->send(200, "text/plain", JSON_PARSE_ERROR);
      return;
    }
    editByServer = true;
    Serial.println("/update-user");
    if(updatesByServer())
      request->send(200, "text/plain", "{\"success\":\"1\"}");
    else
      request->send(200, "text/plain", "{\"success\":\"0\",\"error\":\"user not found\"}");
  });
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
}

void startServer() {
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, apPass);
  delay(1000);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler());
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("content-type"));
  routes();
  // ws.onEvent(onWsEvent);
  // server.addHandler(&ws);
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
