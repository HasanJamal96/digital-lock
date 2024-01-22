/*
TODO:
    Interface following
      - Ethernet shield 
    - Web interface
Done:
    - Local server to setup WiFi
    - Wi-Fi

*/


#include "config.h"
#include <WiFi.h>
#include <WebServer.h>
#include "advance_webpages.h"

// For Ethernet and Websocket
#include <WebSocketClient.h>
using namespace net;

IPAddress apIP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);
WebSocketClient client;




//////////////////////////////////////////////  Websocket Client Callback Functions


void websocketCallback() {
  client.onOpen([](WebSocket &ws) {
    isWebsocketConnected = true;
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server Websocket] Connected\n");
    #endif
    const auto protocol = ws.getProtocol();
    #if(DEBUG == true && DEBUG_SERVER == true)
      if (protocol) {
        Serial.printf("[Advance Server Websocket] Client protocol: ");
        Serial.println(protocol);
      }
    #endif
    // const char message[]{"Hello from Arduino client!"};
    // ws.send(WebSocket::DataType::TEXT, message, strlen(message));
  });

  client.onMessage([](WebSocket &ws, const WebSocket::DataType dataType, const char *msg, uint16_t len) {
    switch (dataType) {
      case WebSocket::DataType::TEXT:
        #if(DEBUG == true && DEBUG_SERVER == true)
          Serial.printf("[Advance Server Websocket] Received: %s\n", msg);
        #endif
        break;
      default:
        break;
    }
    // ws.send(dataType, message, length);
  });

  client.onClose([](WebSocket &ws, const WebSocket::CloseCode, const char *msg, uint16_t len) {
    isWebsocketConnected = false;
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server Websocket] Disconnected\n");
    #endif
  });
}





void routes() {
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", HTML);
  });

  server.on("/jquery.js", HTTP_GET, []() {
    server.send_P(200, "text/javascript", JS);
  });

  server.on("/connect-wifi/ssid/{}/pass/{}", HTTP_POST, []() {
    String sid = server.arg("ssid");
    String pas = server.arg("pass");
    strcpy(wifiName, sid.c_str());
    strcpy(wifiPass, pas.c_str());
    WiFi.disconnect();
    if(strlen(wifiName) > 0)
      forceReconnect = true;

    server.send_P(200, "text/plain", "Ok");
  });

  server.on("/scan-wifi", HTTP_GET, []() {
    #if(DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Scanning for nearby WiFi\n");
    #endif
    server.send(200, "text/plain", "Ok");
    WiFi.scanNetworks(true);
  });

  server.on("/get-wifi-scan-result", HTTP_GET, []() {
    server.send(200, "text/plain", scanResult);
  });
  server.on("/connection-status", HTTP_GET, []() {
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
    server.send(200, "text/plain", InternetConnectionStatus);
  });
}


void startServer() {
  WiFi.softAPConfig(apIP, gateway, subnet);
  WiFi.softAP(apName, apPass);
  routes();
  server.begin();
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Started");
  #endif
}



void stopServer() {
  server.close();
  WiFi.softAPdisconnect();
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Stopped");
  #endif
}


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
//   uint8_t res = info;
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
    WiFi.begin(wifiName, wifiPass);
    #if (DEBUG == true && DEBUG_SERVER == true)
      Serial.printf("[Advance Server] Connecting to WiFi %s\n", wifiName);
    #endif
  }
  connectionStartTime = millis();
}



void serverLoop() {
  if(!wifiConnected) {
    if((millis() - connectionStartTime >= RETRY_AFTER) || forceReconnect) {
      if(forceReconnect)
        forceReconnect = false;
      connectWiFi();
    }
  }
  if(isClientConnected)
    server.handleClient();
}


void initServer() {
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Initializing");
  #endif

  Ethernet.init();
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.onEvent(internetCallback);
  // WiFi.onEvent(internetDisconnectCallback, SYSTEM_EVENT_STA_DISCONNECTED);

  
  String mac = WiFi.macAddress();
  mac.replace(":","");
  strcpy(deviceIdForServer, mac.c_str());

  // websocketCallback();
  
  #if (DEBUG == true && DEBUG_SERVER == true)
    Serial.println("[Advance Server] Initialized");
  #endif
}
