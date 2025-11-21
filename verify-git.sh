#!/bin/bash

echo "=== Git 仓库状态检查 ==="
echo ""

echo "1. 远程仓库配置:"
git remote -v
echo ""

echo "2. 当前分支:"
git branch -a
echo ""

echo "3. 最近的提交:"
git log --oneline -3
echo ""

echo "4. 推送状态:"
git status
echo ""

echo "=== 验证完成 ==="
echo ""
echo "GitHub 仓库地址: https://github.com/diablowu/ESP32-serial-web"
echo ""
echo "如果推送成功，您可以访问上述链接查看代码。"
