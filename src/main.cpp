#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"
#include "ConfigPortal.h"

// --- Globals ---
ConfigManager configManager;
ConfigPortal* configPortal = nullptr;
AsyncWebServer* server = nullptr;
AsyncWebSocket* ws = nullptr;

DeviceConfig currentConfig;
bool inConfigMode = false;
unsigned long configModeStartTime = 0;

// --- WebSocket Event Handler ---
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      // Handle data received from WebSocket -> Send to Serial
      if (len > 0) {
        Serial.write(data, len);
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
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

void startNormalMode() {
  Serial.println("\n=== Starting Normal Mode ===");
  
  // 加载配置
  currentConfig = configManager.getConfig();
  
  // 初始化串口（使用配置的波特率）
  Serial.end();
  Serial.begin(currentConfig.serial_baud_rate);
  delay(100);
  
  Serial.println("--- ESP32 WebSocket Serial Bridge ---");
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
    
    // 初始化WebSocket服务器
    ws = new AsyncWebSocket("/ws");
    ws->onEvent(onWsEvent);
    
    server = new AsyncWebServer(80);
    server->addHandler(ws);
    server->begin();
    
    Serial.println("WebSocket server started");
    Serial.printf("WebSocket URL: ws://%s/ws\n", WiFi.localIP().toString().c_str());
    
    if (strlen(currentConfig.websocket_url) > 0) {
      Serial.printf("Remote WebSocket URL: %s\n", currentConfig.websocket_url);
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
  Serial.println("Version: 2.0 with Web Configuration");
  
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
      delay(5000);  // 等待5秒让用户看到成功消息
      ESP.restart();
    }
    
    // 可选：超时检查（30分钟后自动退出配置模式）
    if (millis() - configModeStartTime > 1800000) {
      Serial.println("Configuration timeout, restarting...");
      ESP.restart();
    }
  } else {
    // 正常模式循环
    if (WiFi.status() == WL_CONNECTED && ws) {
      if (currentConfig.simulate_serial) {
        // 模拟数据生成
        static unsigned long lastSimTime = 0;
        if (millis() - lastSimTime > 1000) {
          lastSimTime = millis();
          String simData = "Simulated Data: " + String(random(0, 1000)) + "\n";
          ws->textAll(simData);
          Serial.print("Generated: " + simData);
        }
      } else {
        // 处理 Serial -> WebSocket
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
            ws->binaryAll(buffer, count);
          }
        }
      }

      // 清理WebSocket客户端
      ws->cleanupClients();
    } else {
      // WiFi断开，尝试重连
      static unsigned long lastReconnectAttempt = 0;
      if (millis() - lastReconnectAttempt > 10000) {
        Serial.println("WiFi disconnected, attempting to reconnect...");
        WiFi.reconnect();
        lastReconnectAttempt = millis();
      }
    }
  }
}
