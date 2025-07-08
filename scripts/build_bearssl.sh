#!/bin/bash

# BearSSL 构建脚本
set -e

print_info() { echo "ℹ️  $1" >&2; }
print_success() { echo "✅ $1" >&2; }
print_error() { echo "❌ $1" >&2; }

# 检查依赖
if ! command -v arm-linux-gnueabihf-gcc &> /dev/null; then
    print_error "ARM交叉编译工具链未安装"
    exit 1
fi

print_info "开始构建 BearSSL..."

# 下载并编译 BearSSL
cd /tmp && \
wget -q https://bearssl.org/bearssl-0.6.tar.gz && \
tar -xf bearssl-0.6.tar.gz && \
cd bearssl-0.6 && \
make clean && \
make CC=arm-linux-gnueabihf-gcc build/libbearssl.a && \
mkdir -p /opt/bearssl && \
cp inc/*.h /opt/bearssl/ && \
cp build/libbearssl.a /opt/bearssl/ && \
cd / && \
rm -rf /tmp/bearssl-0.6* && \
print_success "BearSSL 构建完成: /opt/bearssl/"

echo "/opt/bearssl" 