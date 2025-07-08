# QuickJS 集成说明

本项目已成功集成 QuickJS JavaScript 引擎，可以在 C 程序中执行 JavaScript 代码。

## 文件说明

- `main.c` - 集成了 QuickJS 的主程序
- `test.js` - JavaScript 测试文件
- `tmp_install_quickjs.sh` - QuickJS 临时安装脚本
- `Makefile` - 更新的编译配置

## 安装步骤

### 1. 在容器中安装 QuickJS

```bash
# 给安装脚本执行权限
chmod +x tmp_install_quickjs.sh

# 运行安装脚本
./tmp_install_quickjs.sh
```

安装脚本会自动：
- 安装必要的依赖包
- 下载并编译 QuickJS
- 安装到 `/tmp/quickjs_install` 目录
- 设置环境变量

### 2. 设置环境变量

安装完成后，设置环境变量：

```bash
export QUICKJS_ROOT=/tmp/quickjs_install
export PKG_CONFIG_PATH=/tmp/quickjs_install/lib/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=/tmp/quickjs_install/lib:$LD_LIBRARY_PATH
export PATH=/tmp/quickjs_install/bin:$PATH
```

## 编译和运行

### 使用 Makefile

```bash
# 安装 QuickJS
make install-quickjs

# 测试 QuickJS 安装
make test-quickjs

# 编译程序
make all

# 运行本地版本
make run

# 运行 ARMv7 版本
make run-arm
```

### 手动编译

```bash
# 本地编译
gcc -Wall -Wextra -O2 -std=c99 -I/tmp/quickjs_install/include \
    -o demo main.c -L/tmp/quickjs_install/lib -lquickjs -lm

# ARMv7 交叉编译
arm-linux-gnueabihf-gcc -Wall -Wextra -O2 -std=c99 \
    -march=armv7-a -mfpu=neon -mfloat-abi=hard \
    -I/tmp/quickjs_install/include \
    -o demo_armv7 main.c -L/tmp/quickjs_install/lib -lquickjs -lm
```

## 功能特性

### C 语言功能
- 数学运算（加法、乘法、幂运算）
- 字符串处理（反转字符串）
- 基本的程序结构

### JavaScript 功能
- 执行 `test.js` 文件
- 数学运算测试
- 字符串处理测试
- 数组操作测试
- 对象操作测试
- 错误处理

### 集成特性
- 在 C 程序中嵌入 JavaScript 引擎
- 文件读取和执行
- 错误处理和异常捕获
- 内存管理

## 测试 JavaScript 文件

`test.js` 包含以下测试：

1. **数学运算测试**
   - 基本算术运算
   - 幂运算
   - 数学函数（sqrt, PI）

2. **字符串处理测试**
   - 大小写转换
   - 字符串长度
   - 字符串反转

3. **数组操作测试**
   - 数组求和
   - 数组映射
   - 数组过滤

4. **对象操作测试**
   - JSON 序列化
   - 对象键值获取

## 输出示例

程序运行时会显示：

```
=== ARMv7 Demo Program with QuickJS ===
编译时间: Jan 01 2024 12:00:00
目标架构: ARMv7

C 语言数学运算测试:
10 + 5 = 15
10 * 5 = 50
2.00 ^ 3.00 = 8.00

C 语言字符串处理测试:
原始字符串: Hello ARMv7!
反转后: !7vRA olleH

=== QuickJS 测试 ===
执行 test.js 文件:
=== QuickJS 测试脚本 ===
执行时间: 2024/1/1 12:00:00

--- 数学运算测试 ---
10 + 5 = 15
10 * 5 = 50
10 ** 5 = 100000
Math.sqrt(10) = 3.1622776601683795
Math.PI = 3.141592653589793

--- 字符串处理测试 ---
原始字符串: Hello QuickJS!
大写: HELLO QUICKJS!
小写: hello quickjs!
长度: 13
反转: !SJkciuQ olleH

--- 数组操作测试 ---
原始数组: [1, 2, 3, 4, 5]
数组和: 15
数组平方: [1, 4, 9, 16, 25]
偶数过滤: [2, 4]

--- 对象操作测试 ---
对象: {
  "name": "QuickJS",
  "version": "2021-03-27",
  "features": ["ES2020", "Modules", "BigInt"]
}
对象键: ["name", "version", "features"]
对象值: ["QuickJS", "2021-03-27", ["ES2020", "Modules", "BigInt"]]

=== 测试完成 ===
JavaScript 执行成功!

程序执行完成!
```

## 注意事项

1. **内存管理**: 程序会自动管理 QuickJS 的内存分配和释放
2. **错误处理**: JavaScript 执行错误会被捕获并显示
3. **文件依赖**: 确保 `test.js` 文件在程序运行目录中
4. **临时安装**: QuickJS 安装在临时目录，重启后需要重新安装

## 故障排除

### 编译错误
- 确保 QuickJS 已正确安装
- 检查环境变量设置
- 验证头文件和库文件路径

### 运行时错误
- 检查 `test.js` 文件是否存在
- 确认 QuickJS 库文件可访问
- 查看错误输出信息

### ARMv7 交叉编译
- 确保安装了 ARM 交叉编译工具链
- 验证 QuickJS 是否支持目标架构 