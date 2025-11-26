#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// 配置参数结构体
struct DeviceConfig {
    char wifi_ssid[32];
    char wifi_password[64];
    char websocket_url[128];
    uint32_t serial_baud_rate;
    bool simulate_serial; // 是否模拟串口数据
    bool configured;  // 标记是否已配置
};

// 配置管理类
class ConfigManager {
public:
    ConfigManager();
    
    // 加载配置
    bool loadConfig();
    
    // 保存配置
    bool saveConfig(const DeviceConfig& config);
    
    // 获取当前配置
    DeviceConfig getConfig() const;
    
    // 检查是否已配置
    bool isConfigured() const;
    
    // 重置配置
    void resetConfig();
    
    // 获取默认配置
    static DeviceConfig getDefaultConfig();

private:
    DeviceConfig config;
    static const char* NAMESPACE;
    static const char* CONFIG_VERSION;
};

#endif // CONFIG_H
