#!/bin/bash

# post_js.sh - 将JS文件内容发送到post_exec.js执行
# 用法: ./post_js.sh <js文件路径>

# 检查参数
if [ $# -eq 0 ]; then
    echo "用法: $0 <js文件路径>"
    echo "示例: $0 test.js"
    exit 1
fi

# 获取JS文件路径
JS_FILE="$1"

# 检查文件是否存在
if [ ! -f "$JS_FILE" ]; then
    echo "文件不存在: $JS_FILE"
    exit 1
fi

# 检查文件扩展名
if [[ ! "$JS_FILE" =~ \.js$ ]]; then
    echo "警告: 文件 '$JS_FILE' 不是.js文件"
fi

# 读取文件内容
JS_CONTENT=$(cat "$JS_FILE")

# # 发送POST请求
# echo "正在发送文件 '$JS_FILE' 到 post_exec.js..."
# echo "文件大小: $(wc -c < "$JS_FILE") 字节"

# 使用curl发送POST请求，直接发送原始内容
curl -X POST -H "Content-Type: application/x-www-form-urlencoded" --data-urlencode code@"$JS_FILE" http://localhost:8080/post_exec.js

echo ""
# echo "请求完成" 