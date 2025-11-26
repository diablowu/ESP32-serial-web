#include "Config.h"
#include <Preferences.h>

const char* ConfigManager::NAMESPACE = "device_config";
const char* ConfigManager::CONFIG_VERSION = "v1.0"; // Change this to force config reset

ConfigManager::ConfigManager() {
    config = getDefaultConfig();
}

DeviceConfig ConfigManager::getDefaultConfig() {
    DeviceConfig defaultConfig;
    strcpy(defaultConfig.wifi_ssid, "");
    strcpy(defaultConfig.wifi_password, "");
    strcpy(defaultConfig.websocket_url, "ws://192.168.1.100/ws");
    defaultConfig.serial_baud_rate = 115200;
    defaultConfig.simulate_serial = false;
    defaultConfig.configured = false;
    return defaultConfig;
}

bool ConfigManager::loadConfig() {
    Preferences preferences;
    
    // Open in RW mode to allow clearing if version mismatch
    if (!preferences.begin(NAMESPACE, false)) {
        Serial.println("Failed to open preferences");
        return false;
    }
    
    // Check config version
    String storedVersion = preferences.getString("version", "");
    if (storedVersion != CONFIG_VERSION) {
        Serial.printf("Config version mismatch (Stored: %s, Current: %s). Resetting config.\n", storedVersion.c_str(), CONFIG_VERSION);
        preferences.clear();
        preferences.putString("version", CONFIG_VERSION);
        preferences.end();
        config = getDefaultConfig();
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
    config.simulate_serial = preferences.getBool("sim_serial", false);
    
    preferences.end();
    
    Serial.println("Configuration loaded successfully");
    Serial.printf("WiFi SSID: %s\n", config.wifi_ssid);
    Serial.printf("WebSocket URL: %s\n", config.websocket_url);
    Serial.printf("Baud Rate: %d\n", config.serial_baud_rate);
    Serial.printf("Simulate Serial: %s\n", config.simulate_serial ? "Yes" : "No");
    
    return true;
}

bool ConfigManager::saveConfig(const DeviceConfig& newConfig) {
    Preferences preferences;
    
    if (!preferences.begin(NAMESPACE, false)) {  // 读写模式
        Serial.println("Failed to open preferences for writing");
        return false;
    }
    
    // 保存配置
    preferences.putString("version", CONFIG_VERSION); // Ensure version is saved
    preferences.putString("wifi_ssid", newConfig.wifi_ssid);
    preferences.putString("wifi_pwd", newConfig.wifi_password);
    preferences.putString("ws_url", newConfig.websocket_url);
    preferences.putUInt("baud_rate", newConfig.serial_baud_rate);
    preferences.putBool("sim_serial", newConfig.simulate_serial);
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
        preferences.putString("version", CONFIG_VERSION); // Keep version after reset? Or clear it too? 
        // If we clear it, next boot will see mismatch (empty vs v1.0) and clear again (no harm).
        // But better to set it so next boot doesn't complain if we just want to reset settings but keep "firmware compatibility".
        // However, resetConfig usually means "factory reset".
        // Let's just clear.
        preferences.end();
        Serial.println("Configuration reset");
    }
    
    config = getDefaultConfig();
}
