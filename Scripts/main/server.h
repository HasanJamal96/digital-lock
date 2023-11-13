#include "config.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

IPAddress apIP(8,8,8,8);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");



void routes();
bool updatesByServer();

void websocketLoop() {
  ws.cleanupClients();
}




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
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          // sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if(info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < len; i++) {
          // sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
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
    if(request->url() == "/connecttest.txt" || request->url() == "/redirect" || request->url() == "/generate_204" || request->url() == "/fwlink" || request->url() == "/hotspot-detect.html")
      request->redirect("http://8.8.8.8/");
    else
      request->send_P(200, "text/html", "");
  }
};


void initServer() {
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Initializing");
  #endif
  
  WiFi.mode(WIFI_MODE_APSTA);
  
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Initialized");
  #endif
}


void routes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", "");
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
  server.on("/update-system-info",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(systemInfo, rec);
    if(error) {
      request->send(400, "text/plain", "{\"success\":\"0\",\"error\":\"unable to parse json\"}}");
      return;
    }
    systemByServer = true;
    if(updatesByServer())
      request->send(200, "text/plain", "{\"success\":\"1\"}}");
    else
      request->send(200, "text/plain", "{\"success\":\"0\",\"error\":\"unable to update system info\"}}");
  });
  server.on("/new-user",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(newUser, rec);
    if(error) {
      request->send(400, "text/plain", "{\"success\":\"0\",\"error\":\"unable to parse json\"}}");
      return;
    }
    addByServer = true;
    if(updatesByServer())
      request->send(200, "text/plain", "{\"success\":\"1\"}}");
    else
      request->send(200, "text/plain", "{\"success\":\"0\",\"error\":\"unable to add new user\"}}");
  });
  server.on("/delete-user",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(newUser, rec);
    if(error) {
      request->send(400, "text/plain", "{\"success\":\"0\",\"error\":\"unable to parse json\"}}");
      return;
    }
    deleteByServer = false;
    if(updatesByServer())
      request->send(200, "text/plain", "{\"success\":\"1\"}}");
    else
      request->send(200, "text/plain", "{\"success\":\"0\",\"error\":\"user not found\"}}");
  });
  server.on("/update-user",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec = "";
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    DeserializationError error = deserializeJson(newUser, rec);
    if(error) {
      request->send(400, "text/plain", "{\"success\":\"0\",\"error\":\"unable to parse json\"}}");
      return;
    }
    editByServer = true;
    if(updatesByServer())
      request->send(200, "text/plain", "{\"success\":\"1\"}}");
    else
      request->send(200, "text/plain", "{\"success\":\"0\",\"error\":\"user not found\"}}");
  });
}

void startServer() {
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, apPass);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("content-type"));
  routes();
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Started");
  #endif
}

void closeServer() {
  server.end();
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Server] Closed");
  #endif
}


void restartServer() {
  closeServer();
  startServer();
}