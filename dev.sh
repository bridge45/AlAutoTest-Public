#!/bin/bash

# 本地开发脚本 - 快速编译测试
# 使用方法: ./dev.sh [命令] [选项]

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# 打印函数
print_info() { echo -e "${BLUE}ℹ️  $1${NC}"; }
print_success() { echo -e "${GREEN}✅ $1${NC}"; }
print_warning() { echo -e "${YELLOW}⚠️  $1${NC}"; }
print_error() { echo -e "${RED}❌ $1${NC}"; }
print_debug() { echo -e "${CYAN}🐛 $1${NC}"; }

# 显示帮助
show_help() {
    echo "本地开发脚本"
    echo ""
    echo "使用方法: $0 [命令] [选项]"
    echo ""
    echo "命令:"
    echo "  build    编译程序 (默认)"
    echo "  test     运行测试"
    echo "  debug    调试模式"
    echo "  watch    监视文件变化自动编译"
    echo "  clean    清理文件"
    echo "  install  安装依赖"
    echo "  help     显示帮助"
    echo ""
    echo "选项:"
    echo "  --arm    编译ARM版本 (需要交叉编译工具链)"
    echo "  --native 编译本地版本"
    echo "  --debug  调试模式编译"
    echo ""
    echo "示例:"
    echo "  $0                    # 编译本地版本"
    echo "  $0 build --arm        # 编译ARM版本"
    echo "  $0 test --native      # 测试本地版本"
    echo "  $0 watch              # 监视模式"
}

# 检查依赖
check_dependencies() {
    local target=$1
    
    if [ "$target" = "arm" ]; then
        if ! command -v arm-linux-gnueabihf-gcc &> /dev/null; then
            print_error "ARM交叉编译工具链未安装"
            print_info "请运行: $0 install"
            return 1
        fi
    else
        if ! command -v gcc &> /dev/null; then
            print_error "GCC编译器未安装"
            print_info "请安装GCC编译器"
            return 1
        fi
    fi
    
    return 0
}

# 安装依赖
install_deps() {
    print_info "安装开发依赖..."
    
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux
        sudo apt-get update -qq
        sudo apt-get install -y gcc make qemu-user-static
        if [ "$1" = "arm" ]; then
            sudo apt-get install -y gcc-arm-linux-gnueabihf
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        if command -v brew &> /dev/null; then
            brew install gcc make qemu
            if [ "$1" = "arm" ]; then
                brew install arm-linux-gnueabihf-binutils
            fi
        else
            print_error "请先安装Homebrew"
            return 1
        fi
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        # Windows (Git Bash)
        print_warning "Windows环境，请手动安装:"
        print_info "1. 安装MinGW-w64"
        print_info "2. 或使用WSL"
        return 1
    fi
    
    print_success "依赖安装完成"
}

# 编译
compile() {
    local target=${1:-native}
    local debug=${2:-false}
    
    print_info "编译 $target 版本..."
    
    local cc="gcc"
    local output="demo"
    local cflags="-Wall -Wextra -std=c99"
    
    if [ "$target" = "arm" ]; then
        cc="arm-linux-gnueabihf-gcc"
        output="demo_armv7"
        cflags="$cflags -march=armv7-a -mfpu=neon -mfloat-abi=hard"
    fi
    
    if [ "$debug" = "true" ]; then
        cflags="$cflags -g -O0 -DDEBUG"
        output="${output}_debug"
    else
        cflags="$cflags -O2"
    fi
    
    print_debug "编译器: $cc"
    print_debug "输出文件: $output"
    print_debug "编译选项: $cflags"
    
    $cc $cflags -o $output main.c -lm
    
    print_success "编译完成: $output"
    echo "文件大小: $(stat -c%s $output 2>/dev/null || stat -f%z $output 2>/dev/null) 字节"
}

# 测试
test_program() {
    local target=${1:-native}
    local output="demo"
    
    if [ "$target" = "arm" ]; then
        output="demo_armv7"
    fi
    
    if [ ! -f "$output" ]; then
        print_error "可执行文件不存在: $output"
        print_info "请先运行: $0 build --$target"
        return 1
    fi
    
    print_info "测试 $target 版本..."
    
    chmod +x "$output"
    
    if [ "$target" = "arm" ]; then
        # ARM版本使用QEMU测试
        if command -v qemu-arm &> /dev/null; then
            timeout 10s qemu-arm -L /usr/arm-linux-gnueabihf "$output" || {
                print_warning "程序执行完成或超时"
            }
        else
            print_warning "QEMU未安装，无法测试ARM版本"
            print_info "请安装QEMU: sudo apt-get install qemu-user-static"
        fi
    else
        # 本地版本直接运行
        timeout 10s "./$output" || {
            print_warning "程序执行完成或超时"
        }
    fi
    
    print_success "测试完成"
}

# 调试
debug_program() {
    local target=${1:-native}
    local output="demo_debug"
    
    if [ "$target" = "arm" ]; then
        output="demo_armv7_debug"
    fi
    
    if [ ! -f "$output" ]; then
        print_info "编译调试版本..."
        compile $target true
    fi
    
    print_info "调试 $target 版本..."
    
    if [ "$target" = "arm" ]; then
        if command -v qemu-arm &> /dev/null; then
            print_info "使用QEMU调试ARM版本..."
            qemu-arm -L /usr/arm-linux-gnueabihf -g 1234 "$output" &
            local qemu_pid=$!
            sleep 1
            print_info "QEMU调试端口: 1234"
            print_info "可以使用GDB连接: gdb-multiarch -ex 'target remote localhost:1234' $output"
            wait $qemu_pid
        else
            print_error "QEMU未安装，无法调试ARM版本"
        fi
    else
        print_info "使用GDB调试本地版本..."
        gdb "$output"
    fi
}

# 监视模式
watch_mode() {
    print_info "监视模式启动 (Ctrl+C 退出)..."
    print_info "监视文件: main.c"
    
    local last_modified=0
    
    while true; do
        local current_modified=$(stat -c%Y main.c 2>/dev/null || stat -f%m main.c 2>/dev/null)
        
        if [ "$current_modified" != "$last_modified" ]; then
            echo ""
            print_info "检测到文件变化，重新编译..."
            compile native false
            test_program native
            last_modified=$current_modified
        fi
        
        sleep 1
    done
}

# 清理
clean() {
    print_info "清理构建文件..."
    rm -f demo demo_debug demo_armv7 demo_armv7_debug
    print_success "清理完成"
}

# 主函数
main() {
    local command=${1:-build}
    local target="native"
    local debug="false"
    
    # 解析参数
    shift
    while [[ $# -gt 0 ]]; do
        case $1 in
            --arm)
                target="arm"
                shift
                ;;
            --native)
                target="native"
                shift
                ;;
            --debug)
                debug="true"
                shift
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    case $command in
        build)
            if check_dependencies $target; then
                compile $target $debug
            fi
            ;;
        test)
            if check_dependencies $target; then
                test_program $target
            fi
            ;;
        debug)
            if check_dependencies $target; then
                debug_program $target
            fi
            ;;
        watch)
            if check_dependencies $target; then
                watch_mode
            fi
            ;;
        clean)
            clean
            ;;
        install)
            install_deps $target
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            print_error "未知命令: $command"
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@" 