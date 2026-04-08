cmake_minimum_required(VERSION 3.10)
project(netserver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 网络库核心源文件
set(NETSERVER_SOURCES
    src/Acceptor.cpp
    src/Buffer.cpp
    src/Channel.cpp
    src/Connection.cpp
    src/Epoll.cpp
    src/EventLoop.cpp
    src/InetAddress.cpp
    src/Socket.cpp
    src/TcpServer.cpp
    src/Timestamp.cpp
    src/ThreadPool.cpp
    src/Config.cpp
    src/ServerStats.cpp
    src/EchoServer.cpp
)

# 静态库
add_library(netserver STATIC ${NETSERVER_SOURCES})
target_include_directories(netserver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(netserver PUBLIC pthread)

# 可执行文件
add_executable(echoserver src/main_echoserver.cpp)
target_link_libraries(echoserver PRIVATE netserver)

add_executable(client src/client.cpp)

add_executable(benchmark src/benchmark.cpp)

# 构建时复制配置文件到 build 目录，便于直接运行
add_custom_command(TARGET echoserver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/config/config.conf
        $<TARGET_FILE_DIR:echoserver>/config.conf
)

# 安装
install(TARGETS echoserver client benchmark RUNTIME DESTINATION bin)
install(FILES config/config.conf DESTINATION share/netserver/config)
