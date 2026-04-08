# Reactor

`Reactor` 是一个基于 `Reactor` 模型、`Epoll` 和线程池实现的轻量级 C++ 网络服务器示例项目，包含事件驱动网络层、连接管理、长度头协议解析、业务线程池分发以及基础运行统计能力。

项目以 Echo Server 为演示场景，适合作为 Linux 网络编程、Reactor 模型、多线程 I/O 处理和服务端工程化组织方式的学习与展示项目。

## 项目特性

- 基于 `Epoll` 的事件驱动 I/O 模型
- 主从 `Reactor` 结构，支持多个 I/O 线程
- 业务线程池处理消息逻辑，降低 I/O 线程阻塞风险
- 连接生命周期管理与空闲超时清理
- 基于 4 字节长度头的消息协议，支持粘包与拆包处理
- 内置连接数、消息数和 QPS 等运行统计
- 支持配置文件启动与命令行参数启动
- 提供简单客户端与压测程序，便于联调和性能验证

## 目录结构

```text
Reactor/
├── CMakeLists.txt
├── makefile
├── config/
│   └── config.conf
├── include/
│   ├── Acceptor.h
│   ├── Buffer.h
│   ├── Channel.h
│   ├── Config.h
│   ├── Connection.h
│   ├── EchoServer.h
│   ├── Epoll.h
│   ├── EventLoop.h
│   ├── InetAddress.h
│   ├── ServerStats.h
│   ├── Socket.h
│   ├── TcpServer.h
│   ├── ThreadPool.h
│   └── Timestamp.h
├── src/
│   ├── main_echoserver.cpp
│   ├── client.cpp
│   ├── benchmark.cpp
│   └── ...
└── scripts/
    ├── quick_test.sh
    └── stress_test.sh
```

## 编译环境

- 操作系统：Linux
- 编译器：`g++`
- 语言标准：`C++17`
- 构建工具：`CMake` 或 `make`

> 项目依赖 `epoll`、`pthread` 等 Linux 环境能力，不适用于 Windows 原生运行。

## 编译方式

### 使用 CMake

```bash
mkdir build
cd build
cmake ..
make
```

编译完成后会生成以下可执行文件：

- `echoserver`：服务端程序
- `client`：简单客户端程序
- `benchmark`：单连接压测程序

### 使用 Makefile

```bash
make
```

## 运行方式

### 方式一：使用配置文件启动

如果使用 CMake 构建，`config/config.conf` 会在构建后自动复制到可执行文件目录。

```bash
./echoserver config.conf
```

也可以显式指定配置文件路径：

```bash
./echoserver config/config.conf
```

### 方式二：使用命令行参数启动

```bash
./echoserver 0.0.0.0 5085
```

### 客户端连接示例

```bash
./client 127.0.0.1 5085
```

## 配置文件说明

配置文件路径：`config/config.conf`

```ini
ip=0.0.0.0
port=5085
io_threads=3
work_threads=2
conn_timeout=10
timer_interval=5
stats_interval=10
```

参数说明：

- `ip`：服务监听地址
- `port`：服务监听端口
- `io_threads`：I/O 线程数量
- `work_threads`：业务线程数量
- `conn_timeout`：连接空闲超时时间，单位为秒
- `timer_interval`：定时器检查周期，单位为秒
- `stats_interval`：统计信息输出周期，单位为秒

## 协议说明

项目默认采用长度头协议：

- 前 4 字节：无符号整数，表示消息体长度，网络字节序
- 后续 N 字节：消息体内容

该协议可以有效处理 TCP 通信中的粘包和拆包问题。

## 示例业务命令

当前 `EchoServer` 支持以下示例命令：

- `PING`：返回 `PONG`
- `TIME`：返回当前服务器时间
- `UPPER hello`：返回 `HELLO`
- 其他文本：返回带 `[echo]` 前缀的原消息

## 压力测试

### 单连接压测

```bash
./benchmark 127.0.0.1 5085 100000
```

### 多客户端并发压测

```bash
chmod +x scripts/stress_test.sh
./scripts/stress_test.sh 127.0.0.1 5085 10 5000
```

脚本会自动查找 `build/` 或项目根目录下的 `echoserver` 与 `benchmark` 可执行文件，并输出聚合吞吐结果。

## 核心模块

- `EventLoop`：事件循环、任务唤醒与定时任务调度
- `Epoll` / `Channel`：I/O 事件注册与分发
- `TcpServer` / `Acceptor`：监听、接入与连接分配
- `Connection` / `Buffer`：连接收发、协议解析与发送缓冲
- `ThreadPool`：业务任务异步执行
- `Config`：配置文件解析与启动参数处理
- `ServerStats`：连接数、消息数与 QPS 统计
- `EchoServer`：示例业务层实现

## 适用场景

该项目适合用于：

- C++ Linux 网络编程学习
- Reactor 模型与高并发服务端设计练习
- 面试项目展示
- 轻量级服务端框架原型验证

## 说明

本项目当前以教学示例和工程练习为主要目标，重点在于网络模型、模块拆分和服务端基础能力实现，而非完整生产级中间件封装。
