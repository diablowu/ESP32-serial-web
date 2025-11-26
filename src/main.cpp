#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include "Config.h"
#include "ConfigPortal.h"

// --- Globals ---
ConfigManager configManager;
ConfigPortal* configPortal = nullptr;
AsyncWebServer* server = nullptr; // Used for Config Portal only
WebSocketsClient webSocket;

DeviceConfig currentConfig;
bool inConfigMode = false;
unsigned long configModeStartTime = 0;

// --- WebSocket Event Handler ---
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WSc] Disconnected!");
      break;
    case WStype_CONNECTED:
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      break;
    case WStype_TEXT:
      // Serial.printf("[WSc] get text: %s\n", payload);
      Serial.write(payload, length);
      break;
    case WStype_BIN:
      // Serial.printf("[WSc] get binary length: %u\n", length);
      Serial.write(payload, length);
      break;
    case WStype_PING:
    case WStype_PONG:
    case WStype_ERROR:
      break;
  }
}

void startConfigMode() {
  Serial.println("\n=== Entering Configuration Mode ===");
  inConfigMode = true;
  configModeStartTime = millis();
  
  configPortal = new ConfigPortal(&configManager);
  configPortal->start();
}

void parseUrl(const char* url, String& host, int& port, String& path) {
  String urlStr = String(url);
  
  // Remove ws:// or wss://
  if (urlStr.startsWith("ws://")) {
    urlStr = urlStr.substring(5);
    port = 80;
  } else if (urlStr.startsWith("wss://")) {
    urlStr = urlStr.substring(6);
    port = 443;
  }
  
  int firstSlash = urlStr.indexOf('/');
  int firstColon = urlStr.indexOf(':');
  
  if (firstColon != -1 && (firstSlash == -1 || firstColon < firstSlash)) {
    // Has port
    host = urlStr.substring(0, firstColon);
    if (firstSlash != -1) {
      port = urlStr.substring(firstColon + 1, firstSlash).toInt();
      path = urlStr.substring(firstSlash);
    } else {
      port = urlStr.substring(firstColon + 1).toInt();
      path = "/";
    }
  } else {
    // No port
    if (firstSlash != -1) {
      host = urlStr.substring(0, firstSlash);
      path = urlStr.substring(firstSlash);
    } else {
      host = urlStr;
      path = "/";
    }
  }
}

void startNormalMode() {
  Serial.println("\n=== Starting Normal Mode ===");
  
  // 加载配置
  currentConfig = configManager.getConfig();
  
  // 初始化串口（使用配置的波特率）
  Serial.end();
  Serial.begin(currentConfig.serial_baud_rate);
  delay(100);
  
  Serial.println("--- ESP32 WebSocket Serial Bridge (Client Mode) ---");
  Serial.printf("Serial Baud Rate: %d\n", currentConfig.serial_baud_rate);
  
  // 连接WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(currentConfig.wifi_ssid, currentConfig.wifi_password);
  Serial.printf("Connecting to WiFi: %s", currentConfig.wifi_ssid);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected! IP Address: ");
    Serial.println(WiFi.localIP());
    
    // 连接WebSocket服务器
    String host;
    int port = 80;
    String path;
    
    if (strlen(currentConfig.websocket_url) > 0) {
      parseUrl(currentConfig.websocket_url, host, port, path);
      
      // Generate Device ID from MAC Address
      String mac = WiFi.macAddress();
      mac.replace(":", "");
      String deviceId = "esp32-" + mac;
      
      // Append ID to path
      if (path.indexOf('?') == -1) {
        path += "?id=" + deviceId;
      } else {
        path += "&id=" + deviceId;
      }
      
      Serial.printf("Connecting to WebSocket Server: %s:%d%s\n", host.c_str(), port, path.c_str());
      Serial.printf("Device ID: %s\n", deviceId.c_str());
      
      webSocket.begin(host, port, path);
      webSocket.onEvent(webSocketEvent);
      webSocket.setReconnectInterval(5000);
    } else {
      Serial.println("No WebSocket URL configured!");
    }
    
  } else {
    Serial.println();
    Serial.println("Failed to connect to WiFi!");
    Serial.println("Entering configuration mode...");
    startConfigMode();
  }
}

void setup() {
  // 初始化串口（使用默认波特率）
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== ESP32 WebSocket Serial Bridge ===");
  Serial.println("Version: 2.2 (WebSocket Client)");
  
  // 加载配置
  bool configLoaded = configManager.loadConfig();
  
  if (!configLoaded || !configManager.isConfigured()) {
    Serial.println("No valid configuration found");
    startConfigMode();
  } else {
    Serial.println("Configuration loaded successfully");
    startNormalMode();
  }
}

void loop() {
  if (inConfigMode) {
    // 配置模式循环
    if (configPortal && configPortal->isConfigSubmitted()) {
      Serial.println("Configuration submitted, restarting...");
      delay(5000);
      ESP.restart();
    }
    
    if (millis() - configModeStartTime > 1800000) {
      Serial.println("Configuration timeout, restarting...");
      ESP.restart();
    }
  } else {
    // 正常模式循环
    if (WiFi.status() == WL_CONNECTED) {
      webSocket.loop();
      
      if (currentConfig.simulate_serial) {
        static unsigned long lastSimTime = 0;
        if (millis() - lastSimTime > 1000) {
          lastSimTime = millis();
          String simData = "--- ESP Status ---\n";
          simData += "Uptime: " + String(millis() / 1000) + " s\n";
          simData += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
          simData += "WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
          simData += "Random: " + String(random(0, 1000)) + "\n";
          simData += "------------------\n";
          
          webSocket.sendTXT(simData);
          Serial.print("Generated:\n" + simData);
        }
      } else {
        // Serial -> WebSocket
        if (Serial.available()) {
          uint8_t buffer[256];
          size_t count = 0;
          
          while (Serial.available() && count < sizeof(buffer)) {
            buffer[count++] = Serial.read();
            if (!Serial.available()) {
              delay(1);
            }
          }

          if (count > 0) {
            // Send as Text or Binary? 
            // Simulator sends Stdin as Text. Let's try Text first, or Binary if it contains nulls?
            // For transparency, Binary is safer, but Simulator uses Text for Stdin.
            // Let's use sendTXT for now as requested to match simulator behavior for "text" input.
            // But for generic serial bridge, Binary is better.
            // Given the user referenced the simulator which sends text, I'll send TEXT.
            // However, Serial data might be binary.
            // Let's check if it's printable.
            
            bool isText = true;
            for(size_t i=0; i<count; i++) {
                if (buffer[i] < 9 && buffer[i] != 0) { // Allow 0? No, C-string.
                   // Simple check
                }
            }
            // Just send as TEXT for now, assuming ASCII protocol like the simulator.
            // webSocket.sendTXT(buffer, count); 
            // Note: sendTXT expects null-terminated char* or String, or (uint8_t*, len).
            webSocket.sendTXT(buffer, count);
          }
        }
      }
    } else {
      static unsigned long lastReconnectAttempt = 0;
      if (millis() - lastReconnectAttempt > 10000) {
        Serial.println("WiFi disconnected, attempting to reconnect...");
        WiFi.reconnect();
        lastReconnectAttempt = millis();
      }
    }
  }
}
