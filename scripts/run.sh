#!/bin/bash

# 设置项目根目录（脚本所在位置的上级目录）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# 运行脚本 - 仅编译并执行 ARMv7 版本
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_info() { echo -e "${BLUE}ℹ️  $1${NC}" >&2; }
print_success() { echo -e "${GREEN}✅ $1${NC}" >&2; }
print_warning() { echo -e "${YELLOW}⚠️  $1${NC}" >&2; }
print_error() { echo -e "${RED}❌ $1${NC}" >&2; }

show_help() {
    echo "运行脚本 - 仅编译并执行 ARMv7 版本"
    echo ""
    echo "用法: $0 [--clean] [--help]"
    echo "  --clean    清理构建文件"
    echo "  --help     显示帮助"
    echo ""
    echo "示例:"
    echo "  $0           # 编译并运行 ARMv7 版本"
    echo "  $0 --clean   # 清理后编译运行"
}

check_dependencies() {
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
    return 0
}

compile_arm() {
    print_info "编译 ARMv7 版本..."
    
    # 清理之前的二进制文件
    rm -f build/demo_armv7
    
    local cc="arm-linux-gnueabihf-gcc"
    local output="build/demo_armv7"
    local cflags="-g -Wall -Wextra -O2 -std=c99 -march=armv7-a -mfpu=neon -mfloat-abi=hard -Wno-unused-parameter -Wno-cast-function-type"
    local quickjs_include=""
    local quickjs_lib=""
    local bearssl_include=""
    local bearssl_lib=""
    local microhttpd_include=""
    local microhttpd_lib=""
    
    if [ -n "$QUICKJS_ROOT" ] && [ -d "$QUICKJS_ROOT" ]; then
        if [ -f "$QUICKJS_ROOT/quickjs.h" ] && [ -f "$QUICKJS_ROOT/libquickjs.a" ]; then
            quickjs_include="-I$QUICKJS_ROOT"
            quickjs_lib="-L$QUICKJS_ROOT -lquickjs"
            cflags="$cflags -DQUICKJS_AVAILABLE"
            print_info "检测到 QuickJS: $QUICKJS_ROOT/quickjs.h, $QUICKJS_ROOT/libquickjs.a"
        elif [ -f "$QUICKJS_ROOT/include/quickjs.h" ] && [ -f "$QUICKJS_ROOT/lib/libquickjs.a" ]; then
            quickjs_include="-I$QUICKJS_ROOT/include"
            quickjs_lib="-L$QUICKJS_ROOT/lib -lquickjs"
            cflags="$cflags -DQUICKJS_AVAILABLE"
            print_info "检测到 QuickJS: $QUICKJS_ROOT/include/quickjs.h, $QUICKJS_ROOT/lib/libquickjs.a"
        else
            print_warning "QuickJS 头文件或库文件不存在: $QUICKJS_ROOT/quickjs.h 或 $QUICKJS_ROOT/libquickjs.a，也不存在 $QUICKJS_ROOT/include/quickjs.h 或 $QUICKJS_ROOT/lib/libquickjs.a"
        fi
    else
        print_warning "未检测到 QuickJS 环境，将编译不包含 JavaScript 功能的版本"
    fi
    
    if [ -n "$BEARSSL_ROOT" ] && [ -d "$BEARSSL_ROOT" ]; then
        if [ -f "$BEARSSL_ROOT/bearssl.h" ] && [ -f "$BEARSSL_ROOT/libbearssl.a" ]; then
            bearssl_include="-I$BEARSSL_ROOT"
            bearssl_lib="-L$BEARSSL_ROOT -lbearssl"
            cflags="$cflags -DBEARSSL_AVAILABLE"
            print_info "检测到 BearSSL: $BEARSSL_ROOT/bearssl.h, $BEARSSL_ROOT/libbearssl.a"
        else
            print_warning "BearSSL 头文件或库文件不存在: $BEARSSL_ROOT/bearssl.h 或 $BEARSSL_ROOT/libbearssl.a"
        fi
    else
        print_warning "未检测到 BearSSL 环境，将编译不包含 HTTPS 功能的版本"
    fi
    
    if [ -n "$LIBMICROHTTPD_ROOT" ] && [ -d "$LIBMICROHTTPD_ROOT" ]; then
        if [ -f "$LIBMICROHTTPD_ROOT/include/microhttpd.h" ] && [ -f "$LIBMICROHTTPD_ROOT/lib/libmicrohttpd.a" ]; then
            microhttpd_include="-I$LIBMICROHTTPD_ROOT/include"
            microhttpd_lib="-L$LIBMICROHTTPD_ROOT/lib -lmicrohttpd"
            cflags="$cflags -DMICROHTTPD_AVAILABLE"
            print_info "检测到 libmicrohttpd: $LIBMICROHTTPD_ROOT/include/microhttpd.h, $LIBMICROHTTPD_ROOT/lib/libmicrohttpd.a"
        elif [ -f "$LIBMICROHTTPD_ROOT/microhttpd.h" ] && [ -f "$LIBMICROHTTPD_ROOT/libmicrohttpd.a" ]; then
            microhttpd_include="-I$LIBMICROHTTPD_ROOT"
            microhttpd_lib="-L$LIBMICROHTTPD_ROOT -lmicrohttpd"
            cflags="$cflags -DMICROHTTPD_AVAILABLE"
            print_info "检测到 libmicrohttpd: $LIBMICROHTTPD_ROOT/microhttpd.h, $LIBMICROHTTPD_ROOT/libmicrohttpd.a"
        else
            print_warning "libmicrohttpd 头文件或库文件不存在"
        fi
    else
        print_warning "未检测到 libmicrohttpd 环境，将编译不包含 Web 服务器功能的版本"
    fi
    
    mkdir -p build
    if $cc -static $cflags $quickjs_include $bearssl_include $microhttpd_include -o $output src/main.c "$PROJECT_ROOT/src/resources"/*.c $quickjs_lib $bearssl_lib $microhttpd_lib -lm -lpthread; then
        print_success "编译完成: $output"
        echo "$output"
    else
        print_error "编译失败"
        exit 1
    fi
}

run_program() {
    local output=$1
    print_info "运行程序: $output"
    echo "----------------------------------------"
    chmod +x "$output"
    timeout 30s qemu-arm -L /usr/arm-linux-gnueabihf "$output" || {
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            print_warning "程序执行超时 (30秒)"
        else
            print_warning "程序执行完成，退出码: $exit_code"
        fi
    }
    echo "----------------------------------------"
    print_success "程序执行完成"
}

clean() {
    print_info "清理构建文件..."
    rm -f build/demo_armv7
    print_success "清理完成"
}

# 资源生成函数
generate_resources() {
    # 确保资源目录始终存在
    mkdir -p "$PROJECT_ROOT/src/resources"
    
    # 自动生成资源文件
    if [ -d "$PROJECT_ROOT/third_bin" ] && [ "$(ls -A "$PROJECT_ROOT/third_bin")" ]; then
        rm -f "$PROJECT_ROOT/src/resources"/*.c "$PROJECT_ROOT/src/resources"/resource_list.*
        RESOURCE_LIST_H="$PROJECT_ROOT/src/resources/resource_list.h"
        RESOURCE_LIST_C="$PROJECT_ROOT/src/resources/resource_list.c"

        echo "#pragma once" > "$RESOURCE_LIST_H"
        echo "#include <stddef.h>" >> "$RESOURCE_LIST_H"
        echo "#include <stdlib.h>" >> "$RESOURCE_LIST_H"
        echo "typedef struct { const char* name; const unsigned char* data; size_t len; } Resource;" >> "$RESOURCE_LIST_H"
        echo "void init_resources();" >> "$RESOURCE_LIST_H"
        echo "Resource* get_resources();" >> "$RESOURCE_LIST_H"
        echo "size_t get_resources_count();" >> "$RESOURCE_LIST_H"

        echo "#include \"resource_list.h\"" > "$RESOURCE_LIST_C"
        
        # 先声明所有外部变量
        for f in "$PROJECT_ROOT/third_bin"/*; do
          fname=$(basename "$f")
          arrname=$(echo "$fname" | tr . _ | tr - _)
          echo "extern unsigned char ${arrname}[];" >> "$RESOURCE_LIST_C"
          echo "extern unsigned int ${arrname}_len;" >> "$RESOURCE_LIST_C"
        done
        
        echo "static Resource* resources = NULL;" >> "$RESOURCE_LIST_C"
        echo "static size_t resources_count = 0;" >> "$RESOURCE_LIST_C"
        echo "void init_resources() {" >> "$RESOURCE_LIST_C"
        echo "  if (resources) return;" >> "$RESOURCE_LIST_C"
        echo "  resources_count = 0;" >> "$RESOURCE_LIST_C"
        for f in "$PROJECT_ROOT/third_bin"/*; do
          echo "  resources_count++;" >> "$RESOURCE_LIST_C"
        done
        echo "  resources = malloc(sizeof(Resource) * resources_count);" >> "$RESOURCE_LIST_C"
        echo "  int idx = 0;" >> "$RESOURCE_LIST_C"
        for f in "$PROJECT_ROOT/third_bin"/*; do
          fname=$(basename "$f")
          arrname=$(echo "$fname" | tr . _ | tr - _)
          xxd -i "$f" | sed "s/unsigned char .*\[\]/unsigned char ${arrname}[]/" | sed "s/unsigned int .*_len/unsigned int ${arrname}_len/" > "$PROJECT_ROOT/src/resources/$fname.c"
          echo "  resources[idx].name = \"$fname\";" >> "$RESOURCE_LIST_C"
          echo "  resources[idx].data = ${arrname};" >> "$RESOURCE_LIST_C"
          echo "  resources[idx].len = ${arrname}_len;" >> "$RESOURCE_LIST_C"
          echo "  idx++;" >> "$RESOURCE_LIST_C"
          echo "extern unsigned char ${arrname}[];" >> "$RESOURCE_LIST_H"
          echo "extern unsigned int ${arrname}_len;" >> "$RESOURCE_LIST_H"
        done
        echo "}" >> "$RESOURCE_LIST_C"
        echo "Resource* get_resources() { if (!resources) init_resources(); return resources; }" >> "$RESOURCE_LIST_C"
        echo "size_t get_resources_count() { if (!resources) init_resources(); return resources_count; }" >> "$RESOURCE_LIST_C"
    else
        echo "third_bin 目录不存在或为空，跳过资源打包。"
    fi
}

# 脚本最前面调用
generate_resources

main() {
    local clean_before="false"
    while [[ $# -gt 0 ]]; do
        case $1 in
            --clean)
                clean_before="true"
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
    if ! check_dependencies; then
        exit 1
    fi
    if [ "$clean_before" = "true" ]; then
        clean
    fi
    local output=$(compile_arm)
    run_program $output
}

main "$@" 