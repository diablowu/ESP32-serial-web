#include "ConfigPortal.h"

const char* ConfigPortal::AP_SSID = "ESP32-Config";
const char* ConfigPortal::AP_PASSWORD = "";  // 无密码
const IPAddress ConfigPortal::AP_IP(192, 168, 4, 1);
const IPAddress ConfigPortal::AP_GATEWAY(192, 168, 4, 1);
const IPAddress ConfigPortal::AP_SUBNET(255, 255, 255, 0);

ConfigPortal::ConfigPortal(ConfigManager* configMgr) 
    : configManager(configMgr), server(nullptr), configSubmitted(false) {
}

ConfigPortal::~ConfigPortal() {
    stop();
}

void ConfigPortal::start() {
    Serial.println("\n=== Starting Configuration Portal ===");
    
    // 配置AP模式
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    Serial.printf("AP SSID: %s\n", AP_SSID);
    Serial.printf("AP IP: %s\n", AP_IP.toString().c_str());
    Serial.println("Please connect to the AP and visit http://192.168.4.1");
    
    // 创建Web服务器
    server = new AsyncWebServer(80);
    setupRoutes();
    server->begin();
    
    Serial.println("Configuration portal started");
}

void ConfigPortal::stop() {
    if (server) {
        server->end();
        delete server;
        server = nullptr;
    }
    WiFi.softAPdisconnect(true);
}

void ConfigPortal::setupRoutes() {
    // 主页 - 配置表单
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        request->send(200, "text/html", generateConfigPage());
    });
    
    // 处理配置提交
    server->on("/save", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleConfigSubmit(request);
    });
    
    // 404处理
    server->onNotFound([](AsyncWebServerRequest* request) {
        request->redirect("/");
    });
}

String ConfigPortal::generateConfigPage() {
    DeviceConfig currentConfig = configManager->getConfig();
    
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 配置</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            padding: 40px;
            max-width: 500px;
            width: 100%;
        }
        h1 {
            color: #667eea;
            margin-bottom: 10px;
            font-size: 28px;
            text-align: center;
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 8px;
            color: #333;
            font-weight: 500;
            font-size: 14px;
        }
        input[type="text"],
        input[type="password"],
        input[type="number"] {
            width: 100%;
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            font-size: 14px;
            transition: all 0.3s;
        }
        input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        .hint {
            font-size: 12px;
            color: #999;
            margin-top: 5px;
        }
        button {
            width: 100%;
            padding: 14px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        .success-message {
            display: none;
            background: #d4edda;
            color: #155724;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
            text-align: center;
        }
        .icon {
            font-size: 48px;
            text-align: center;
            margin-bottom: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="icon">⚙️</div>
        <h1>ESP32 设备配置</h1>
        <p class="subtitle">请配置您的设备参数</p>
        
        <div id="successMessage" class="success-message">
            ✓ 配置保存成功！设备将在 <span id="countdown">5</span> 秒后重启...
        </div>
        
        <form id="configForm" action="/save" method="POST">
            <div class="form-group">
                <label for="wifi_ssid">WiFi 名称 (SSID) *</label>
                <input type="text" id="wifi_ssid" name="wifi_ssid" 
                       value=")rawliteral" + String(currentConfig.wifi_ssid) + R"rawliteral(" 
                       required maxlength="31">
                <div class="hint">请输入要连接的WiFi网络名称</div>
            </div>
            
            <div class="form-group">
                <label for="wifi_password">WiFi 密码 *</label>
                <input type="password" id="wifi_password" name="wifi_password" 
                       value=")rawliteral" + String(currentConfig.wifi_password) + R"rawliteral(" 
                       required maxlength="63">
                <div class="hint">请输入WiFi密码</div>
            </div>
            
            <div class="form-group">
                <label for="websocket_url">WebSocket URL</label>
                <input type="text" id="websocket_url" name="websocket_url" 
                       value=")rawliteral" + String(currentConfig.websocket_url) + R"rawliteral(" 
                       maxlength="127">
                <div class="hint">例如: ws://192.168.1.100/ws</div>
            </div>
            
            <div class="form-group">
                <label for="baud_rate">串口波特率 *</label>
                <input type="number" id="baud_rate" name="baud_rate" 
                       value=")rawliteral" + String(currentConfig.serial_baud_rate) + R"rawliteral(" 
                       required min="9600" max="921600">
                <div class="hint">常用值: 9600, 115200, 921600</div>
            </div>

            <div class="form-group">
                <label style="display: flex; align-items: center; cursor: pointer;">
                    <input type="checkbox" id="simulate_serial" name="simulate_serial" 
                           value="true" )rawliteral" + String(currentConfig.simulate_serial ? "checked" : "") + R"rawliteral(
                           style="width: auto; margin-right: 10px;">
                    启用模拟串口数据
                </label>
                <div class="hint">开启后将生成随机数据发送到WebSocket，不读取实际串口</div>
            </div>
            
            <button type="submit">💾 保存配置</button>
        </form>
    </div>
    
    <script>
        document.getElementById('configForm').addEventListener('submit', function(e) {
            e.preventDefault();
            
            const formData = new FormData(this);
            
            fetch('/save', {
                method: 'POST',
                body: new URLSearchParams(formData)
            })
            .then(response => response.text())
            .then(data => {
                document.getElementById('configForm').style.display = 'none';
                document.getElementById('successMessage').style.display = 'block';
                
                let countdown = 5;
                const countdownEl = document.getElementById('countdown');
                
                const timer = setInterval(() => {
                    countdown--;
                    countdownEl.textContent = countdown;
                    if (countdown <= 0) {
                        clearInterval(timer);
                    }
                }, 1000);
            })
            .catch(error => {
                alert('保存失败: ' + error);
            });
        });
    </script>
</body>
</html>
)rawliteral";
    
    return html;
}

void ConfigPortal::handleConfigSubmit(AsyncWebServerRequest* request) {
    Serial.println("Received configuration submission");
    
    DeviceConfig newConfig;
    
    // 获取表单数据
    if (request->hasParam("wifi_ssid", true)) {
        String ssid = request->getParam("wifi_ssid", true)->value();
        ssid.toCharArray(newConfig.wifi_ssid, sizeof(newConfig.wifi_ssid));
    }
    
    if (request->hasParam("wifi_password", true)) {
        String password = request->getParam("wifi_password", true)->value();
        password.toCharArray(newConfig.wifi_password, sizeof(newConfig.wifi_password));
    }
    
    if (request->hasParam("websocket_url", true)) {
        String url = request->getParam("websocket_url", true)->value();
        url.toCharArray(newConfig.websocket_url, sizeof(newConfig.websocket_url));
    }
    
    if (request->hasParam("baud_rate", true)) {
        newConfig.serial_baud_rate = request->getParam("baud_rate", true)->value().toInt();
    }

    if (request->hasParam("simulate_serial", true)) {
        newConfig.simulate_serial = request->getParam("simulate_serial", true)->value() == "true";
    } else {
        newConfig.simulate_serial = false;
    }
    
    newConfig.configured = true;
    
    // 保存配置
    if (configManager->saveConfig(newConfig)) {
        submittedConfig = newConfig;
        configSubmitted = true;
        
        request->send(200, "text/plain", "OK");
        
        Serial.println("Configuration saved, will restart in 5 seconds");
    } else {
        request->send(500, "text/plain", "Failed to save configuration");
    }
}

bool ConfigPortal::isConfigSubmitted() const {
    return configSubmitted;
}

DeviceConfig ConfigPortal::getSubmittedConfig() const {
    return submittedConfig;
}
