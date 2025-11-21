#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// --- Configuration ---
// Replace with your network credentials
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#define SERIAL_BAUD_RATE 115200
#define WS_PORT 80
#define WS_ENDPOINT "/ws"

// --- Globals ---
AsyncWebServer server(WS_PORT);
AsyncWebSocket ws(WS_ENDPOINT);

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
      // We assume text or binary data is just raw bytes to be forwarded
      if (len > 0) {
        Serial.write(data, len);
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void setup() {
  // 1. Initialize Serial
  Serial.begin(SERIAL_BAUD_RATE);
  // Wait a bit for serial to stabilize
  delay(1000);
  Serial.println("\n\n--- ESP32 WebSocket Serial Bridge ---");

  // 2. Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());

  // 3. Initialize WebSocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // 4. Start Server
  server.begin();
  Serial.println("WebSocket server started");
}

void loop() {
  // 5. Handle Serial -> WebSocket
  if (Serial.available()) {
    // Read available bytes into a buffer
    uint8_t buffer[256];
    size_t count = 0;
    
    while (Serial.available() && count < sizeof(buffer)) {
      buffer[count++] = Serial.read();
      // Small delay to allow buffer to fill if more data is coming quickly
      if (!Serial.available()) {
        delay(1);
      }
    }

    if (count > 0) {
      // Broadcast to all connected WebSocket clients
      ws.binaryAll(buffer, count);
    }
  }

  // Clean up WebSocket clients
  ws.cleanupClients();
}
