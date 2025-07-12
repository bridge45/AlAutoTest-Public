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
    wget \
    pkg-config \
    libtool \
    autoconf \
    automake \
    && rm -rf /var/lib/apt/lists/*

# 安装 QuickJS
RUN cd /tmp && \
    wget -q https://bellard.org/quickjs/quickjs-2021-03-27.tar.xz && \
    tar -xf quickjs-2021-03-27.tar.xz && \
    cd quickjs-2021-03-27 && \
    make clean && \
    make CC=arm-linux-gnueabihf-gcc libquickjs.a && \
    mkdir -p /opt/quickjs && \
    cp quickjs.h /opt/quickjs/ && \
    cp quickjs-libc.h /opt/quickjs/ && \
    cp libquickjs.a /opt/quickjs/ && \
    cd / && \
    rm -rf /tmp/quickjs-2021-03-27* && \
    echo "QuickJS 安装完成"

# 安装 libmicrohttpd
RUN echo "正在安装 libmicrohttpd..." && \
    wget -q https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.75.tar.gz && \
    tar -xf libmicrohttpd-0.9.75.tar.gz && \
    cd libmicrohttpd-0.9.75 && \
    ./configure --host=arm-linux-gnueabihf --prefix=/opt/libmicrohttpd --disable-shared --enable-static && \
    make && \
    make install && \
    cd / && \
    rm -rf /tmp/libmicrohttpd-0.9.76* && \
    echo "libmicrohttpd 安装完成"

# 设置工作目录
WORKDIR /workspace


# 设置环境变量
ENV ARM_CC="arm-linux-gnueabihf-gcc"
ENV ARM_CXX="arm-linux-gnueabihf-g++"
ENV QUICKJS_ROOT="/opt/quickjs"
ENV LIBMICROHTTPD_ROOT="/opt/libmicrohttpd"
ENV PATH="/opt/quickjs:/opt/libmicrohttpd/bin:$PATH"

# 默认命令
CMD ["/bin/bash"] 