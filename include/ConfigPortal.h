#ifndef CONFIG_PORTAL_H
#define CONFIG_PORTAL_H

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif
#include <ESPAsyncWebServer.h>
#include "Config.h"

class ConfigPortal {
public:
    ConfigPortal(ConfigManager* configMgr);
    ~ConfigPortal();
    
    // 启动配置门户（AP模式）
    void start();
    
    // 停止配置门户
    void stop();
    
    // 检查是否有配置提交
    bool isConfigSubmitted() const;
    
    // 获取提交的配置
    DeviceConfig getSubmittedConfig() const;

private:
    ConfigManager* configManager;
    AsyncWebServer* server;
    bool configSubmitted;
    DeviceConfig submittedConfig;
    
    // AP配置
    static const char* AP_SSID;
    static const char* AP_PASSWORD;
    static const IPAddress AP_IP;
    static const IPAddress AP_GATEWAY;
    static const IPAddress AP_SUBNET;
    
    // 设置路由
    void setupRoutes();
    
    // 生成HTML页面
    String generateConfigPage();
    
    // 处理配置提交
    void handleConfigSubmit(AsyncWebServerRequest* request);
};

#endif // CONFIG_PORTAL_H
