#!/bin/bash

# 运行脚本 - 编译并执行程序
# 使用方法: ./run.sh [选项]

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# 打印函数
print_info() { echo -e "${BLUE}ℹ️  $1${NC}" >&2; }
print_success() { echo -e "${GREEN}✅ $1${NC}" >&2; }
print_warning() { echo -e "${YELLOW}⚠️  $1${NC}" >&2; }
print_error() { echo -e "${RED}❌ $1${NC}" >&2; }
print_debug() { echo -e "${CYAN}🐛 $1${NC}" >&2; }

# 显示帮助
show_help() {
    echo "运行脚本 - 编译并执行程序"
    echo ""
    echo "使用方法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  --arm      编译并运行ARM版本"
    echo "  --native   编译并运行本地版本 (默认)"
    echo "  --debug    调试模式编译并运行"
    echo "  --clean    运行前清理旧文件"
    echo "  --watch    监视模式 (文件变化时自动重新编译运行)"
    echo "  --help     显示帮助"
    echo ""
    echo "示例:"
    echo "  $0                    # 编译并运行本地版本"
    echo "  $0 --arm              # 编译并运行ARM版本"
    echo "  $0 --debug            # 调试模式"
    echo "  $0 --clean            # 清理后运行"
    echo "  $0 --watch            # 监视模式"
}

# 检查依赖
check_dependencies() {
    local target=$1
    
    if [ "$target" = "arm" ]; then
        if ! command -v arm-linux-gnueabihf-gcc &> /dev/null; then
            print_error "ARM交叉编译工具链未安装"
            print_info "请安装: sudo apt-get install gcc-arm-linux-gnueabihf"
            return 1
        fi
        if ! command -v qemu-arm &> /dev/null; then
            print_error "QEMU未安装，无法运行ARM版本"
            print_info "请安装: sudo apt-get install qemu-user-static"
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
    echo "文件大小: $(stat -c%s $output 2>/dev/null || stat -f%z $output 2>/dev/null) 字节" >&2
    
    # 返回输出文件名
    echo "$output"
}

# 运行程序
run_program() {
    local output=$1
    local target=${2:-native}
    
    print_info "运行程序: $output"
    echo "----------------------------------------"
    
    chmod +x "$output"
    
    if [ "$target" = "arm" ]; then
        # ARM版本使用QEMU运行
        timeout 30s qemu-arm -L /usr/arm-linux-gnueabihf "$output" || {
            local exit_code=$?
            if [ $exit_code -eq 124 ]; then
                print_warning "程序执行超时 (30秒)"
            else
                print_warning "程序执行完成，退出码: $exit_code"
            fi
        }
    else
        # 本地版本直接运行
        timeout 30s "./$output" || {
            local exit_code=$?
            if [ $exit_code -eq 124 ]; then
                print_warning "程序执行超时 (30秒)"
            else
                print_warning "程序执行完成，退出码: $exit_code"
            fi
        }
    fi
    
    echo "----------------------------------------"
    print_success "程序执行完成"
}

# 清理
clean() {
    print_info "清理构建文件..."
    rm -f demo demo_debug demo_armv7 demo_armv7_debug
    print_success "清理完成"
}

# 监视模式
watch_mode() {
    local target=${1:-native}
    local debug=${2:-false}
    
    print_info "监视模式启动 (Ctrl+C 退出)..."
    print_info "监视文件: main.c"
    print_info "目标平台: $target"
    if [ "$debug" = "true" ]; then
        print_info "调试模式: 启用"
    fi
    
    local last_modified=0
    
    while true; do
        local current_modified=$(stat -c%Y main.c 2>/dev/null || stat -f%m main.c 2>/dev/null)
        
        if [ "$current_modified" != "$last_modified" ]; then
            echo ""
            print_info "检测到文件变化，重新编译运行..."
            
            # 编译
            local output=$(compile $target $debug)
            
            # 运行
            run_program $output $target
            
            last_modified=$current_modified
            echo ""
        fi
        
        sleep 1
    done
}

# 主函数
main() {
    local target="native"
    local debug="false"
    local clean_before="false"
    local watch_mode="false"
    
    # 解析参数
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
            --clean)
                clean_before="true"
                shift
                ;;
            --watch)
                watch_mode="true"
                shift
                ;;
            --help|-h)
                show_help
                exit 0
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # 检查依赖
    if ! check_dependencies $target; then
        exit 1
    fi
    
    # 清理
    if [ "$clean_before" = "true" ]; then
        clean
    fi
    
    # 监视模式
    if [ "$watch_mode" = "true" ]; then
        watch_mode $target $debug
        exit 0
    fi
    
    # 编译
    local output=$(compile $target $debug)
    
    # 运行
    run_program $output $target
}

# 运行主函数
main "$@" 