#pragma once
#include <string>
#include <cstdint>

/**
 * 简易配置：从文件或默认值加载 ip/port/线程数/超时等
 */
class Config
{
public:
    std::string ip{"0.0.0.0"};
    uint16_t port{5085};
    int io_threads{3};
    int work_threads{2};
    int conn_timeout{10};
    int timer_interval{5};
    int stats_interval{10};  // 统计打印间隔（秒），0 不打印

    /** 从文件加载，失败则使用默认值 */
    bool load(const std::string& path);

    /** 从命令行覆盖：./echoserver [config.conf] 或 ./echoserver ip port */
    void parseArgs(int argc, char* argv[]);
};
