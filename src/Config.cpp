#include "Config.h"
#include <Preferences.h>

const char* ConfigManager::NAMESPACE = "device_config";

ConfigManager::ConfigManager() {
    config = getDefaultConfig();
}

DeviceConfig ConfigManager::getDefaultConfig() {
    DeviceConfig defaultConfig;
    strcpy(defaultConfig.wifi_ssid, "");
    strcpy(defaultConfig.wifi_password, "");
    strcpy(defaultConfig.websocket_url, "ws://192.168.1.100/ws");
    defaultConfig.serial_baud_rate = 115200;
    defaultConfig.configured = false;
    return defaultConfig;
}

bool ConfigManager::loadConfig() {
    Preferences preferences;
    
    if (!preferences.begin(NAMESPACE, true)) {  // 只读模式
        Serial.println("Failed to open preferences");
        return false;
    }
    
    // 检查是否已配置
    config.configured = preferences.getBool("configured", false);
    
    if (!config.configured) {
        preferences.end();
        config = getDefaultConfig();
        return false;
    }
    
    // 加载配置
    preferences.getString("wifi_ssid", config.wifi_ssid, sizeof(config.wifi_ssid));
    preferences.getString("wifi_pwd", config.wifi_password, sizeof(config.wifi_password));
    preferences.getString("ws_url", config.websocket_url, sizeof(config.websocket_url));
    config.serial_baud_rate = preferences.getUInt("baud_rate", 115200);
    
    preferences.end();
    
    Serial.println("Configuration loaded successfully");
    Serial.printf("WiFi SSID: %s\n", config.wifi_ssid);
    Serial.printf("WebSocket URL: %s\n", config.websocket_url);
    Serial.printf("Baud Rate: %d\n", config.serial_baud_rate);
    
    return true;
}

bool ConfigManager::saveConfig(const DeviceConfig& newConfig) {
    Preferences preferences;
    
    if (!preferences.begin(NAMESPACE, false)) {  // 读写模式
        Serial.println("Failed to open preferences for writing");
        return false;
    }
    
    // 保存配置
    preferences.putString("wifi_ssid", newConfig.wifi_ssid);
    preferences.putString("wifi_pwd", newConfig.wifi_password);
    preferences.putString("ws_url", newConfig.websocket_url);
    preferences.putUInt("baud_rate", newConfig.serial_baud_rate);
    preferences.putBool("configured", true);
    
    preferences.end();
    
    config = newConfig;
    config.configured = true;
    
    Serial.println("Configuration saved successfully");
    
    return true;
}

DeviceConfig ConfigManager::getConfig() const {
    return config;
}

bool ConfigManager::isConfigured() const {
    return config.configured;
}

void ConfigManager::resetConfig() {
    Preferences preferences;
    
    if (preferences.begin(NAMESPACE, false)) {
        preferences.clear();
        preferences.end();
        Serial.println("Configuration reset");
    }
    
    config = getDefaultConfig();
}
