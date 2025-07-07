# ARMv7 自动编译 Demo

这是一个简单的C语言demo项目，演示如何分离本地开发和远程构建流程。

## 🎯 开发理念

**本地开发** → **快速迭代** → **远程构建** → **发布版本**

- 🏠 **本地开发**：快速编译、测试、调试
- ☁️ **远程构建**：自动化构建、发布、分发

## 项目特性

- ⚡ **本地快速开发**：秒级编译测试
- 🔄 **文件监视**：自动重新编译
- 🐛 **本地调试**：支持GDB调试
- ☁️ **云端构建**：GitHub Actions自动构建
- 📦 **自动发布**：生成ARMv7二进制文件

## 文件结构

```
.
├── main.c                    # 主程序源代码
├── main_windows.c           # Windows版本源代码
├── Makefile                 # 传统编译脚本
├── build.sh                 # 简单构建脚本
├── dev.sh                   # 本地开发脚本 ⭐
├── .github/workflows/       # GitHub Actions工作流
│   ├── build-and-test.yml  # 优化版构建流程
│   └── debug.yml           # 调试工作流
└── README.md               # 项目说明
```

## 🚀 快速开始

### 1. 本地开发（推荐）

```bash
# 给脚本添加执行权限
chmod +x dev.sh

# 安装依赖
./dev.sh install

# 快速编译和测试
./dev.sh

# 监视模式（自动重新编译）
./dev.sh watch

# 调试模式
./dev.sh debug
```

### 2. 本地开发命令

```bash
# 编译本地版本
./dev.sh build

# 编译ARM版本
./dev.sh build --arm

# 测试本地版本
./dev.sh test

# 测试ARM版本
./dev.sh test --arm

# 调试模式编译
./dev.sh build --debug

# 监视文件变化
./dev.sh watch

# 清理文件
./dev.sh clean
```

## 🔄 推荐开发流程

### 阶段1：本地开发
```bash
# 1. 启动监视模式
./dev.sh watch

# 2. 编辑 main.c
# 3. 自动编译和测试
# 4. 重复直到功能完成
```

### 阶段2：提交和构建
```bash
# 1. 提交代码
git add .
git commit -m "新功能完成"
git push

# 2. GitHub Actions自动构建
# 3. 下载ARMv7二进制文件
```

## ☁️ GitHub Actions 自动构建

当代码推送到 `main` 分支时，GitHub Actions会自动：

1. ⚡ **快速安装** ARM交叉编译工具链
2. 🔨 **编译** ARMv7二进制文件
3. 🧪 **测试** 程序执行
4. 📦 **打包** 发布文件
5. 📤 **上传** 构建产物

### 查看构建结果

1. 进入GitHub仓库页面
2. 点击 "Actions" 标签页
3. 查看最新的构建工作流
4. 下载构建产物

## 📋 开发工具对比

| 工具 | 用途 | 速度 | 适用场景 |
|------|------|------|----------|
| `./dev.sh` | 本地开发 | ⚡⚡⚡ | 日常开发、调试 |
| `./build.sh` | 简单构建 | ⚡⚡ | 快速构建 |
| `make` | 传统构建 | ⚡ | 兼容性构建 |
| GitHub Actions | 远程构建 | 🐌 | 发布版本 |

## 🛠️ 环境要求

### 本地开发环境

**Linux/macOS:**
```bash
# 安装依赖
sudo apt-get install gcc make qemu-user-static
sudo apt-get install gcc-arm-linux-gnueabihf  # ARM交叉编译
```

**Windows:**
```bash
# 推荐使用WSL或Git Bash
# 安装MinGW-w64或使用WSL
```

### 云端构建环境

- ✅ GitHub Actions (Ubuntu 20.04)
- ✅ 自动安装所有依赖
- ✅ 无需本地配置

## 📊 性能对比

| 操作 | 本地开发 | GitHub Actions |
|------|----------|----------------|
| 编译时间 | 0.1-0.5秒 | 30-60秒 |
| 测试时间 | 0.1-1秒 | 5-10秒 |
| 调试便利性 | ⭐⭐⭐⭐⭐ | ⭐⭐ |
| 发布便利性 | ⭐⭐ | ⭐⭐⭐⭐⭐ |

## 🎯 最佳实践

### 本地开发
1. 使用 `./dev.sh watch` 启动监视模式
2. 频繁修改和测试
3. 使用 `./dev.sh debug` 调试问题
4. 功能完成后再提交

### 远程构建
1. 提交代码触发自动构建
2. 检查构建日志
3. 下载发布版本
4. 在目标设备上测试

## 🔧 故障排除

### 本地编译问题
```bash
# 检查依赖
./dev.sh install

# 清理重新编译
./dev.sh clean
./dev.sh build
```

### GitHub Actions问题
1. 检查Actions页面错误日志
2. 确认代码语法正确
3. 查看构建产物是否生成

## 📝 许可证

MIT License