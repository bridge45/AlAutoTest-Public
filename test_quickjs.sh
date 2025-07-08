#!/bin/bash

# QuickJS 安装测试脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() { echo -e "${BLUE}ℹ️  $1${NC}"; }
print_success() { echo -e "${GREEN}✅ $1${NC}"; }
print_warning() { echo -e "${YELLOW}⚠️  $1${NC}"; }
print_error() { echo -e "${RED}❌ $1${NC}"; }

echo "=== QuickJS 安装测试 ==="

# 检查环境变量
if [ -n "$QUICKJS_ROOT" ]; then
    print_success "QUICKJS_ROOT 环境变量已设置: $QUICKJS_ROOT"
else
    print_warning "QUICKJS_ROOT 环境变量未设置"
fi

# 检查 QuickJS 目录
if [ -d "$QUICKJS_ROOT" ]; then
    print_success "QuickJS 目录存在: $QUICKJS_ROOT"
    
    # 检查关键文件
    files=("quickjs.h" "quickjs-libc.h" "libquickjs.a" "qjs" "qjsc")
    for file in "${files[@]}"; do
        if [ -f "$QUICKJS_ROOT/$file" ]; then
            print_success "文件存在: $file"
        else
            print_error "文件缺失: $file"
        fi
    done
else
    print_error "QuickJS 目录不存在: $QUICKJS_ROOT"
fi

# 测试 qjs 命令
if command -v qjs &> /dev/null; then
    print_success "qjs 命令可用"
    echo "版本信息:"
    qjs --help 2>&1 | grep "QuickJS version" || echo "无法获取版本信息"
else
    print_error "qjs 命令不可用"
fi

# 测试 JavaScript 执行
if [ -f "test.js" ]; then
    print_info "测试 JavaScript 执行..."
    if qjs test.js 2>/dev/null; then
        print_success "JavaScript 执行测试通过"
    else
        print_error "JavaScript 执行测试失败"
    fi
else
    print_warning "test.js 文件不存在，跳过 JavaScript 执行测试"
fi

echo ""
echo "=== 测试完成 ===" 