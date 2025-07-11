# ARMv7 自动编译 Demo

这是一个简单的C语言demo项目，演示如何分离本地开发和远程构建流程。

docker-compose build  

docker-compose up -d 

进入容器
docker exec -it armv7-dev bash

目标设备启动
 /tmp/demo_armv7 --port 8083  --wdir /config/worker
  /tmp/demo_armv7 --port 8083  --wdir /tmp


qemu-arm -L /usr/arm-linux-gnueabihf build/demo_armv7 