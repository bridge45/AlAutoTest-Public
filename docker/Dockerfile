FROM ubuntu:20.04

# 设置环境变量
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai

# 安装ARM交叉编译工具链和开发工具
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    binutils-arm-linux-gnueabihf \
    qemu-user-static \
    qemu-user \
    make \
    vim \
    git \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /workspace

# 复制项目文件
COPY . .

# 设置权限
RUN chmod +x *.sh

# 设置环境变量
ENV ARM_CC="arm-linux-gnueabihf-gcc"
ENV ARM_CXX="arm-linux-gnueabihf-g++"

# 默认命令
CMD ["/bin/bash"] 