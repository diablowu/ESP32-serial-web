# ESP32 WebSocket 串口桥接

这个项目实现了一个ESP32 WebSocket到串口的透明桥接功能，允许Web客户端通过WebSocket与连接到ESP32串口的设备进行通信。

## 功能特性

- ✅ WebSocket服务器（端口80，路径`/ws`）
- ✅ 双向数据传输：WebSocket ↔ Serial
- ✅ 支持文本和二进制数据
- ✅ 多客户端连接支持
- ✅ 基于PlatformIO构建

## 硬件要求

- ESP32开发板
- USB数据线（用于烧录和调试）
- 可选：连接到ESP32串口的其他MCU设备

## 软件要求

- [PlatformIO](https://platformio.org/)
- Python 3.x（PlatformIO依赖）

## 配置

### WiFi设置

在烧录之前，需要修改 `src/main.cpp` 中的WiFi凭据：

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

### 串口设置

默认串口配置：
- 波特率：115200
- 端口：UART0（USB串口）

如需修改，请编辑 `src/main.cpp` 中的 `SERIAL_BAUD_RATE` 定义。

## 构建和烧录

### 1. 构建项目

```bash
cd /home/master/works/ESP32-serial-web
platformio run
```

### 2. 烧录到ESP32

```bash
platformio run --target upload
```

### 3. 监控串口输出

```bash
platformio device monitor
```

## 使用方法

### 1. 连接到WiFi

ESP32启动后会自动连接到配置的WiFi网络。通过串口监视器可以看到分配的IP地址。

### 2. WebSocket连接

使用WebSocket客户端连接到：

```
ws://<ESP32_IP_ADDRESS>/ws
```

### 3. 测试连接

#### 使用wscat（命令行工具）

```bash
# 安装wscat
npm install -g wscat

# 连接到ESP32
wscat -c ws://192.168.1.100/ws

# 发送消息
> Hello ESP32
```

#### 使用HTML页面

创建一个简单的HTML文件：

```html
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 WebSocket测试</title>
</head>
<body>
    <h1>ESP32 WebSocket串口桥接测试</h1>
    <div>
        <button onclick="connect()">连接</button>
        <button onclick="disconnect()">断开</button>
    </div>
    <div>
        <input type="text" id="message" placeholder="输入消息">
        <button onclick="send()">发送</button>
    </div>
    <div>
        <h3>接收的消息：</h3>
        <pre id="output"></pre>
    </div>

    <script>
        let ws = null;
        
        function connect() {
            ws = new WebSocket('ws://192.168.1.100/ws'); // 修改为你的ESP32 IP
            
            ws.onopen = () => {
                document.getElementById('output').textContent += '已连接\n';
            };
            
            ws.onmessage = (event) => {
                document.getElementById('output').textContent += '收到: ' + event.data + '\n';
            };
            
            ws.onclose = () => {
                document.getElementById('output').textContent += '连接已关闭\n';
            };
        }
        
        function disconnect() {
            if (ws) {
                ws.close();
            }
        }
        
        function send() {
            const msg = document.getElementById('message').value;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(msg);
                document.getElementById('output').textContent += '发送: ' + msg + '\n';
                document.getElementById('message').value = '';
            }
        }
    </script>
</body>
</html>
```

## 数据流

```
Web客户端 (WebSocket) ←→ ESP32 (WebSocket Server) ←→ 串口设备
```

- **WebSocket → Serial**: 从WebSocket接收的数据会立即写入串口
- **Serial → WebSocket**: 从串口读取的数据会广播给所有连接的WebSocket客户端

## 项目结构

```
ESP32-serial-web/
├── src/
│   └── main.cpp           # 主程序代码
├── include/               # 头文件目录
├── lib/                   # 本地库目录
├── test/                  # 测试代码
├── platformio.ini         # PlatformIO配置
└── README.md             # 本文档
```

## 依赖库

- `esphome/AsyncTCP-esphome` - 异步TCP库
- `ottowinter/ESPAsyncWebServer-esphome` - 异步Web服务器库

## 故障排除

### 无法连接到WiFi

1. 检查WiFi凭据是否正确
2. 确保ESP32在WiFi信号范围内
3. 检查路由器是否允许新设备连接

### WebSocket连接失败

1. 确认ESP32已成功连接到WiFi
2. 检查IP地址是否正确
3. 确保防火墙没有阻止80端口

### 串口数据丢失

1. 检查波特率设置是否匹配
2. 确认串口连接是否正确
3. 增加缓冲区大小（如果需要）

## 协议支持

本项目实现的是透明串口桥接，不对数据进行解析。如果需要支持特定的协议（如Beacon命令协议），可以在ESP32端添加协议解析逻辑。

## 许可证

本项目基于MIT许可证开源。

## 贡献

欢迎提交问题和拉取请求！
