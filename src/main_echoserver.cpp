/*
 * 程序名：main_echoserver.cpp，回显（EchoServer）服务器入口。
 * 用法：./echoserver <config.conf>  或  ./echoserver <ip> <port>
*/
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include "EchoServer.h"
#include "Config.h"

EchoServer* echoserver = nullptr;

void Stop(int sig)
{
    (void)sig;
    if (echoserver)
    {
        echoserver->Stop();
        delete echoserver;
        echoserver = nullptr;
        printf("echoserver已停止。\n");
    }
    exit(0);
}

int main(int argc, char* argv[])
{
    // 确保在后台/重定向时也能及时看到日志输出。
    setvbuf(stdout, nullptr, _IOLBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    if (argc < 2)
    {
        printf("usage: %s <config.conf>\n", argv[0]);
        printf("   or: %s <ip> <port>\n", argv[0]);
        printf("example: %s config.conf\n", argv[0]);
        printf("example: %s 0.0.0.0 5085\n\n", argv[0]);
        return -1;
    }

    signal(SIGTERM, Stop);
    signal(SIGINT, Stop);

    Config cfg;
    cfg.parseArgs(argc, argv);
    printf("listening %s:%u io=%d work=%d\n", cfg.ip.c_str(), cfg.port, cfg.io_threads, cfg.work_threads);

    echoserver = new EchoServer(cfg);
    echoserver->Start();

    return 0;
}
