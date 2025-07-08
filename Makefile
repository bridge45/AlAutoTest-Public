# ARMv7 交叉编译 Makefile with QuickJS

# 编译器设置
CC = gcc
ARM_CC = arm-linux-gnueabihf-gcc

# QuickJS 安装路径 (从环境变量获取，默认为临时安装路径)
QUICKJS_ROOT ?= /tmp/quickjs_install
QUICKJS_INCLUDE = $(QUICKJS_ROOT)/include
QUICKJS_LIB = $(QUICKJS_ROOT)/lib

# 编译选项
CFLAGS = -Wall -Wextra -O2 -std=c99 -I$(QUICKJS_INCLUDE)
ARM_CFLAGS = -Wall -Wextra -O2 -std=c99 -march=armv7-a -mfpu=neon -mfloat-abi=hard -I$(QUICKJS_INCLUDE)

# 链接选项
LDFLAGS = -L$(QUICKJS_LIB) -lquickjs -lm
ARM_LDFLAGS = -L$(QUICKJS_LIB) -lquickjs -lm

# 目标文件
TARGET = demo
ARM_TARGET = demo_armv7

# 源文件
SOURCES = main.c

# 默认目标
all: $(TARGET) $(ARM_TARGET)

# 本地编译
$(TARGET): $(SOURCES)
	@echo "编译本地版本..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ARMv7 交叉编译
$(ARM_TARGET): $(SOURCES)
	@echo "编译 ARMv7 版本..."
	$(ARM_CC) $(ARM_CFLAGS) -o $@ $^ $(ARM_LDFLAGS)

# 清理
clean:
	rm -f $(TARGET) $(ARM_TARGET)

# 安装依赖 (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y gcc-arm-linux-gnueabihf

# 安装 QuickJS
install-quickjs:
	@echo "安装 QuickJS..."
	chmod +x tmp_install_quickjs.sh
	./tmp_install_quickjs.sh

# 测试 QuickJS
test-quickjs:
	@echo "测试 QuickJS..."
	@if [ -f "$(QUICKJS_ROOT)/bin/qjs" ]; then \
		echo "QuickJS 已安装，版本:"; \
		$(QUICKJS_ROOT)/bin/qjs --version; \
		echo "测试 JavaScript 执行:"; \
		$(QUICKJS_ROOT)/bin/qjs test.js; \
	else \
		echo "QuickJS 未安装，请先运行: make install-quickjs"; \
	fi

# 运行程序
run: $(TARGET)
	@echo "运行本地版本..."
	./$(TARGET)

run-arm: $(ARM_TARGET)
	@echo "运行 ARMv7 版本..."
	./$(ARM_TARGET)

# 显示帮助
help:
	@echo "可用的目标:"
	@echo "  all          - 编译本地和ARMv7版本"
	@echo "  demo         - 编译本地版本"
	@echo "  demo_armv7   - 编译ARMv7版本"
	@echo "  clean        - 清理编译文件"
	@echo "  install-deps - 安装交叉编译工具链"
	@echo "  install-quickjs - 安装 QuickJS"
	@echo "  test-quickjs - 测试 QuickJS 安装"
	@echo "  run          - 运行本地版本"
	@echo "  run-arm      - 运行ARMv7版本"
	@echo "  help         - 显示此帮助信息"
	@echo ""
	@echo "环境变量:"
	@echo "  QUICKJS_ROOT - QuickJS 安装路径 (默认: /tmp/quickjs_install)"

.PHONY: all clean install-deps install-quickjs test-quickjs run run-arm help 