# ARMv7 交叉编译 Makefile

# 编译器设置
CC = gcc
ARM_CC = arm-linux-gnueabihf-gcc

# 编译选项
CFLAGS = -Wall -Wextra -O2 -std=c99
ARM_CFLAGS = -Wall -Wextra -O2 -std=c99 -march=armv7-a -mfpu=neon -mfloat-abi=hard

# 目标文件
TARGET = demo
ARM_TARGET = demo_armv7

# 源文件
SOURCES = main.c

# 默认目标
all: $(TARGET) $(ARM_TARGET)

# 本地编译
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# ARMv7 交叉编译
$(ARM_TARGET): $(SOURCES)
	$(ARM_CC) $(ARM_CFLAGS) -o $@ $^ -lm

# 清理
clean:
	rm -f $(TARGET) $(ARM_TARGET)

# 安装依赖 (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y gcc-arm-linux-gnueabihf

# 显示帮助
help:
	@echo "可用的目标:"
	@echo "  all          - 编译本地和ARMv7版本"
	@echo "  demo         - 编译本地版本"
	@echo "  demo_armv7   - 编译ARMv7版本"
	@echo "  clean        - 清理编译文件"
	@echo "  install-deps - 安装交叉编译工具链"
	@echo "  help         - 显示此帮助信息"

.PHONY: all clean install-deps help 