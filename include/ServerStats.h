#pragma once
#include <atomic>
#include <cstdint>
#include <ctime>
#include <string>
#include <mutex>

/**
 * 服务端统计：连接数、消息数、QPS
 */
class ServerStats
{
public:
    ServerStats();

    void onConnect();
    void onDisconnect();
    void onMessage();

    uint64_t totalConnections() const { return total_conn_; }
    uint64_t currentConnections() const { return current_conn_; }
    uint64_t totalMessages() const { return total_msg_; }

    /** 平均 QPS（自启动以来的总消息数 / 运行秒数） */
    double avgQps() const;
    /** 近期 QPS（最近 stats_interval_ 秒内的消息数 / stats_interval_） */
    double recentQps();
    /** 运行时长（秒） */
    time_t uptime() const;

    /** 打印统计信息到 stdout */
    void print();

    /** 设置统计打印间隔（秒），0 表示不自动打印 */
    void setStatsInterval(int sec) { stats_interval_ = sec; }

private:
    std::atomic<uint64_t> total_conn_{0};
    std::atomic<uint64_t> current_conn_{0};
    std::atomic<uint64_t> total_msg_{0};
    time_t start_time_{0};

    int stats_interval_{10};
    time_t last_print_time_{0};
    uint64_t last_msg_count_{0};
    std::mutex print_mutex_;
};
