#include "ServerStats.h"
#include <cstdio>
#include <unistd.h>

ServerStats::ServerStats()
{
    start_time_ = time(nullptr);
    last_print_time_ = start_time_;
}

void ServerStats::onConnect()
{
    total_conn_++;
    current_conn_++;
}

void ServerStats::onDisconnect()
{
    uint64_t cur = current_conn_;
    if (cur > 0) current_conn_--;  // 避免下溢
}

void ServerStats::onMessage()
{
    total_msg_++;
}

double ServerStats::avgQps() const
{
    time_t sec = uptime();
    if (sec <= 0) return 0.0;
    return static_cast<double>(total_msg_.load()) / sec;
}

double ServerStats::recentQps()
{
    if (stats_interval_ <= 0) return avgQps();
    time_t now = time(nullptr);
    time_t elapsed = now - last_print_time_;
    if (elapsed <= 0) return avgQps();
    uint64_t cur = total_msg_.load();
    uint64_t delta = (cur >= last_msg_count_) ? (cur - last_msg_count_) : 0;
    last_print_time_ = now;
    last_msg_count_ = cur;
    return static_cast<double>(delta) / elapsed;
}

time_t ServerStats::uptime() const
{
    time_t now = time(nullptr);
    return (now > start_time_) ? (now - start_time_) : 0;
}

void ServerStats::print()
{
    if (stats_interval_ <= 0) return;
    std::lock_guard<std::mutex> g(print_mutex_);
    time_t now = time(nullptr);
    if (now - last_print_time_ < stats_interval_ && last_print_time_ != 0) return;
    time_t elapsed = (now > start_time_) ? (now - start_time_) : 0;
    uint64_t cur_msg = total_msg_.load();
    time_t span = (now > last_print_time_) ? (now - last_print_time_) : 1;
    uint64_t delta = (cur_msg >= last_msg_count_) ? (cur_msg - last_msg_count_) : 0;
    double qps = (span > 0) ? static_cast<double>(delta) / span : avgQps();
    last_print_time_ = now;
    last_msg_count_ = cur_msg;

    printf("[Stats] uptime=%lus conn_total=%lu conn_current=%lu msg_total=%lu qps=%.1f\n",
           static_cast<unsigned long>(elapsed),
           static_cast<unsigned long>(total_conn_.load()),
           static_cast<unsigned long>(current_conn_.load()),
           static_cast<unsigned long>(cur_msg),
           qps);
    fflush(stdout);
}
