#!/bin/bash
set -e

cd /home/master/works/ESP32-serial-web

# 配置Git用户信息
git config user.name "ESP32 Developer"
git config user.email "developer@example.com"

# 添加所有文件
git add -A

# 显示状态
echo "=== Git Status ==="
git status

# 创建提交
git commit -m "Initial commit: ESP32 WebSocket Serial Bridge

Features:
- WebSocket server on port 80 (/ws endpoint)
- Bidirectional Serial-WebSocket bridge
- Multi-client support
- PlatformIO build system
- Complete documentation and test tool"

echo ""
echo "=== Commit Created ==="
git log --oneline -1

echo ""
echo "=== Next Steps ==="
echo "To push to GitHub, run:"
echo "git remote add origin https://github.com/YOUR_USERNAME/ESP32-serial-web.git"
echo "git branch -M main"
echo "git push -u origin main"
