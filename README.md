# ARMv7 自动编译 Demo

这是一个简单的C语言demo项目，演示如何使用GitHub Actions自动编译ARMv7目标设备的二进制文件。

## 项目特性

- 🚀 自动ARMv7交叉编译
- 📦 GitHub Actions CI/CD
- 🔧 支持本地和交叉编译
- 📋 包含数学运算和字符串处理示例

## 文件结构

```
.
├── main.c                 # 主程序源代码
├── Makefile              # 编译脚本
├── .github/workflows/    # GitHub Actions工作流
│   └── build.yml        # 自动构建配置
└── README.md            # 项目说明
```

## 本地编译

### 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y gcc-arm-linux-gnueabihf

# 或者使用Makefile
make install-deps
```

### 编译

```bash
# 编译本地版本
make demo

# 编译ARMv7版本
make demo_armv7

# 编译所有版本
make all
```

## GitHub Actions 自动编译

当代码推送到 `main` 或 `master` 分支时，GitHub Actions会自动：

1. 安装ARM交叉编译工具链
2. 编译ARMv7二进制文件
3. 验证二进制文件架构
4. 上传构建产物
5. 创建发布（当推送标签时）

### 查看构建结果

1. 进入GitHub仓库页面
2. 点击 "Actions" 标签
3. 查看最新的构建工作流
4. 下载构建产物

## 使用方法

### 在ARMv7设备上运行

```bash
# 下载二进制文件
chmod +x demo_armv7
./demo_armv7
```

### 预期输出

```
=== ARMv7 Demo Program ===
编译时间: Dec 25 2023 10:30:45
目标架构: ARMv7

数学运算测试:
10 + 5 = 15
10 * 5 = 50
2.00 ^ 3.00 = 8.00

字符串处理测试:
原始字符串: Hello ARMv7!
反转后: !7vMRA olleH

程序执行完成!
```

## 创建发布

要创建GitHub发布：

```bash
# 创建标签
git tag v1.0.0

# 推送标签
git push origin v1.0.0
```

GitHub Actions会自动创建发布并上传二进制文件。

## 技术细节

- **目标架构**: ARMv7-A
- **浮点单元**: NEON
- **浮点ABI**: Hard
- **编译器**: gcc-arm-linux-gnueabihf
- **优化级别**: -O2

## 贡献

欢迎提交Issue和Pull Request！

## 许可证

MIT License