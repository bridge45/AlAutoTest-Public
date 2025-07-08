#!/bin/bash

# QuickJS 临时安装脚本
# 适用于 Ubuntu/Debian 系统

set -e  # 遇到错误立即退出

echo "=== QuickJS 临时安装脚本 ==="
echo "开始安装 QuickJS..."

# 检查系统
if ! command -v gcc &> /dev/null; then
    echo "错误: 未找到 gcc 编译器"
    exit 1
fi

# 安装依赖
echo "安装依赖包..."
# 检查是否有 sudo 权限
if command -v sudo &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        git \
        wget \
        pkg-config \
        libtool \
        autoconf \
        automake
else
    echo "未找到 sudo，尝试直接使用 apt-get..."
    apt-get update
    apt-get install -y \
        build-essential \
        git \
        wget \
        pkg-config \
        libtool \
        autoconf \
        automake
fi

# 创建临时目录
TEMP_DIR="/tmp/quickjs_build"
echo "创建临时目录: $TEMP_DIR"
rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR"
cd "$TEMP_DIR"

# 下载 QuickJS
echo "下载 QuickJS 源码..."
QUICKJS_VERSION="2021-03-27"
QUICKJS_URL="https://bellard.org/quickjs/quickjs-${QUICKJS_VERSION}.tar.xz"

if ! wget -q "$QUICKJS_URL"; then
    echo "下载失败，尝试从 GitHub 获取..."
    git clone --depth 1 https://github.com/bellard/quickjs.git
    cd quickjs
else
    echo "解压 QuickJS 源码..."
    tar -xf "quickjs-${QUICKJS_VERSION}.tar.xz"
    cd "quickjs-${QUICKJS_VERSION}"
fi

# 安装到临时位置
INSTALL_DIR="/tmp/quickjs_install"
echo "安装到临时位置: $INSTALL_DIR"
mkdir -p "$INSTALL_DIR"

# 编译 QuickJS
echo "编译 QuickJS..."
make

# 检查是否需要 ARMv7 交叉编译
if command -v arm-linux-gnueabihf-gcc &> /dev/null; then
    echo "检测到 ARM 交叉编译工具链，编译 ARMv7 版本..."
    make clean
    make CC=arm-linux-gnueabihf-gcc CFLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=hard"
    
    # 创建 ARMv7 版本的库文件
    mkdir -p "$INSTALL_DIR/lib/armv7"
    cp libquickjs.a "$INSTALL_DIR/lib/armv7/"
    echo "ARMv7 版本编译完成"
else
    echo "未检测到 ARM 交叉编译工具链，跳过 ARMv7 版本编译"
fi
echo "安装到临时位置: $INSTALL_DIR"
mkdir -p "$INSTALL_DIR"

# 复制头文件
mkdir -p "$INSTALL_DIR/include"
cp quickjs.h "$INSTALL_DIR/include/"
cp quickjs-libc.h "$INSTALL_DIR/include/"

# 复制库文件
mkdir -p "$INSTALL_DIR/lib"
cp libquickjs.a "$INSTALL_DIR/lib/"

# 复制可执行文件
mkdir -p "$INSTALL_DIR/bin"
cp qjs "$INSTALL_DIR/bin/"
cp qjsc "$INSTALL_DIR/bin/"

# 创建 pkg-config 文件
mkdir -p "$INSTALL_DIR/lib/pkgconfig"
cat > "$INSTALL_DIR/lib/pkgconfig/quickjs.pc" << EOF
prefix=$INSTALL_DIR
exec_prefix=\${prefix}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: QuickJS
Description: QuickJS JavaScript engine
Version: $QUICKJS_VERSION
Libs: -L\${libdir} -lquickjs
Cflags: -I\${includedir}
EOF

# 设置环境变量
echo "设置环境变量..."
export QUICKJS_ROOT="$INSTALL_DIR"
export PKG_CONFIG_PATH="$INSTALL_DIR/lib/pkgconfig:$PKG_CONFIG_PATH"
export LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH"
export PATH="$INSTALL_DIR/bin:$PATH"

# 测试安装
echo "测试 QuickJS 安装..."
if "$INSTALL_DIR/bin/qjs" --help 2>&1 | grep -q "QuickJS version"; then
    echo "QuickJS 安装成功!"
    echo ""
    echo "=== 安装信息 ==="
    echo "安装目录: $INSTALL_DIR"
    echo "头文件: $INSTALL_DIR/include/"
    echo "库文件: $INSTALL_DIR/lib/"
    echo "可执行文件: $INSTALL_DIR/bin/"
    echo ""
    echo "=== 环境变量 ==="
    echo "export QUICKJS_ROOT=$INSTALL_DIR"
    echo "export PKG_CONFIG_PATH=$INSTALL_DIR/lib/pkgconfig:\$PKG_CONFIG_PATH"
    echo "export LD_LIBRARY_PATH=$INSTALL_DIR/lib:\$LD_LIBRARY_PATH"
    echo "export PATH=$INSTALL_DIR/bin:\$PATH"
    echo ""
    echo "=== 测试命令 ==="
    echo "qjs --help  # 显示版本和帮助信息"
    echo "qjs test.js  # 执行 JavaScript 文件"
else
    echo "QuickJS 安装失败!"
    exit 1
fi

echo ""
echo "安装完成! 现在可以编译集成 QuickJS 的 C 程序了。" 