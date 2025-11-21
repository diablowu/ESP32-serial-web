# ESP32 WebSocket 串口桥接

这个项目实现了一个ESP32 WebSocket到串口的透明桥接功能，允许Web客户端通过WebSocket与连接到ESP32串口的设备进行通信。

## 功能特性

- ✅ **Web配置界面** - 首次启动时通过AP模式配置设备（v2.0新增）
- ✅ **WebSocket服务器** - 端口80，路径`/ws`
- ✅ **双向数据传输** - WebSocket ↔ Serial
- ✅ **多客户端连接支持**
- ✅ **配置持久化** - 使用NVS存储配置
- ✅ **自动WiFi重连**
- ✅ **基于PlatformIO构建**

## 硬件要求

- ESP32开发板
- USB数据线（用于烧录和调试）
- 可选：连接到ESP32串口的其他MCU设备

## 软件要求

- [PlatformIO](https://platformio.org/)
- Python 3.x（PlatformIO依赖）

## 快速开始

### 1. 构建和烧录

```bash
cd /home/master/works/ESP32-serial-web
platformio run --target upload
```

### 2. 首次配置

首次烧录后，ESP32会自动进入配置模式：

1. **连接到ESP32的WiFi热点**
   - SSID: `ESP32-Config`
   - 无密码

2. **打开配置页面**
   - 在浏览器中访问: `http://192.168.4.1`

3. **填写配置信息**
   - WiFi名称（SSID）
   - WiFi密码
   - WebSocket URL（可选）
   - 串口波特率（默认115200）

4. **保存配置**
   - 点击"保存配置"按钮
   - ESP32将在5秒后自动重启并连接到配置的WiFi

### 3. 正常使用

配置完成后，ESP32会：
- 自动连接到配置的WiFi网络
- 启动WebSocket服务器
- 使用配置的波特率初始化串口

## 配置界面预览

配置界面采用现代化设计，包含：
- 渐变紫色背景
- 响应式表单
- 实时验证
- 保存成功提示和倒计时

## 使用方法

### WebSocket连接

使用WebSocket客户端连接到：

```
ws://<ESP32_IP_ADDRESS>/ws
```

### 测试工具

项目包含一个精美的WebSocket测试工具 `websocket-test.html`，可以直接在浏览器中打开使用。

#### 使用wscat（命令行工具）

```bash
# 安装wscat
npm install -g wscat

# 连接到ESP32
wscat -c ws://192.168.1.100/ws

# 发送消息
> Hello ESP32
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
│   ├── main.cpp              # 主程序
│   ├── Config.cpp            # 配置管理实现
│   └── ConfigPortal.cpp      # 配置门户实现
├── include/
│   ├── Config.h              # 配置管理头文件
│   └── ConfigPortal.h        # 配置门户头文件
├── lib/                      # 本地库目录
├── test/                     # 测试代码
├── platformio.ini            # PlatformIO配置
├── websocket-test.html       # WebSocket测试工具
└── README.md                 # 本文档
```

## 依赖库

- `esphome/AsyncTCP-esphome` - 异步TCP库
- `ottowinter/ESPAsyncWebServer-esphome` - 异步Web服务器库
- `Preferences` - ESP32内置NVS存储库

## 配置管理

### 重置配置

如果需要重新配置设备，可以：

1. **通过代码重置**：在 `setup()` 中调用 `configManager.resetConfig()`
2. **重新烧录固件**：会保留配置，除非手动清除

### 配置存储

配置保存在ESP32的NVS（非易失性存储）中，包括：
- WiFi SSID和密码
- WebSocket URL
- 串口波特率
- 配置状态标志

## 故障排除

### 无法进入配置模式

1. 确保是首次烧录或配置已被清除
2. 检查串口监视器输出
3. 手动清除NVS分区

### 无法连接到WiFi

1. 检查WiFi凭据是否正确
2. 确保ESP32在WiFi信号范围内
3. 查看串口输出的错误信息
4. 设备会在WiFi连接失败后自动进入配置模式

### WebSocket连接失败

1. 确认ESP32已成功连接到WiFi
2. 检查IP地址是否正确
3. 确保防火墙没有阻止80端口

### 串口数据丢失

1. 检查波特率设置是否匹配
2. 确认串口连接是否正确
3. 增加缓冲区大小（如果需要）

## 高级功能

### 远程WebSocket URL

配置中的"WebSocket URL"字段可用于：
- 记录远程服务器地址
- 未来扩展：ESP32作为WebSocket客户端连接到远程服务器

### 自动重连

ESP32会自动检测WiFi断开并尝试重连，每10秒尝试一次。

### 配置超时

配置模式会在30分钟后自动超时并重启设备，防止设备长时间停留在配置模式。

## 版本历史

### v2.0 (2025-11-21)
- ✨ 新增Web配置界面
- ✨ 支持AP模式配置
- ✨ 配置持久化存储
- ✨ 自动WiFi重连
- ✨ 可配置串口波特率

### v1.0 (2025-11-21)
- 🎉 初始版本
- ✅ WebSocket串口桥接
- ✅ 基础功能实现

## 协议支持

本项目实现的是透明串口桥接，不对数据进行解析。如果需要支持特定的协议（如Beacon命令协议），可以在ESP32端添加协议解析逻辑。

## GitHub仓库

https://github.com/diablowu/ESP32-serial-web

## 许可证

本项目基于MIT许可证开源。

## 贡献

欢迎提交问题和拉取请求！
