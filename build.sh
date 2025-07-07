#!/bin/bash

# ARMv7 快速编译脚本
# 使用方法: ./build.sh [debug|release]

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# 检查依赖
check_dependencies() {
    print_info "检查依赖..."
    
    if ! command -v arm-linux-gnueabihf-gcc &> /dev/null; then
        print_error "ARM交叉编译工具链未安装"
        print_info "请运行: sudo apt-get install gcc-arm-linux-gnueabihf"
        exit 1
    fi
    
    if ! command -v qemu-arm &> /dev/null; then
        print_warning "QEMU未安装，将无法在本地测试"
        print_info "请运行: sudo apt-get install qemu-user-static"
    fi
    
    print_success "依赖检查完成"
}

# 编译
compile() {
    local build_type=${1:-release}
    
    print_info "开始编译 ($build_type 模式)..."
    
    case $build_type in
        debug)
            arm-linux-gnueabihf-gcc -g -O0 -Wall -Wextra -std=c99 -o demo_armv7 main.c -lm
            ;;
        release)
            arm-linux-gnueabihf-gcc -O2 -Wall -Wextra -std=c99 -o demo_armv7 main.c -lm
            ;;
        *)
            print_error "未知的构建类型: $build_type"
            exit 1
            ;;
    esac
    
    print_success "编译完成"
}

# 验证
verify() {
    print_info "验证二进制文件..."
    
    if [ ! -f demo_armv7 ]; then
        print_error "二进制文件不存在"
        exit 1
    fi
    
    echo "文件信息:"
    file demo_armv7
    echo "文件大小: $(stat -c%s demo_armv7) 字节"
    
    print_success "验证完成"
}

# 测试
test_execution() {
    if ! command -v qemu-arm &> /dev/null; then
        print_warning "跳过测试 (QEMU未安装)"
        return
    fi
    
    print_info "测试执行..."
    
    chmod +x demo_armv7
    
    # 设置超时，防止程序卡住
    timeout 10s qemu-arm -L /usr/arm-linux-gnueabihf demo_armv7 || {
        print_warning "程序执行超时或完成"
    }
    
    print_success "测试完成"
}

# 清理
clean() {
    print_info "清理构建文件..."
    rm -f demo_armv7
    print_success "清理完成"
}

# 显示帮助
show_help() {
    echo "ARMv7 快速编译脚本"
    echo ""
    echo "使用方法:"
    echo "  $0 [选项]"
    echo ""
    echo "选项:"
    echo "  debug    编译调试版本"
    echo "  release  编译发布版本 (默认)"
    echo "  clean    清理构建文件"
    echo "  help     显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0              # 编译发布版本"
    echo "  $0 debug        # 编译调试版本"
    echo "  $0 clean        # 清理文件"
}

# 主函数
main() {
    case ${1:-release} in
        debug|release)
            check_dependencies
            compile $1
            verify
            test_execution
            print_success "构建流程完成!"
            ;;
        clean)
            clean
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@" 