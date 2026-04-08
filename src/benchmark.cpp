/**
 * 压测客户端：单连接发送 N 次请求，统计耗时与 QPS。
 * 用法：./benchmark <ip> <port> <count>
 * 示例：./benchmark 127.0.0.1 5085 100000
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>
#include <chrono>
#include <vector>

static bool writen(int fd, const char* data, size_t size)
{
    size_t total = 0;
    while (total < size)
    {
        ssize_t n = send(fd, data + total, size - total, 0);
        if (n > 0) { total += n; continue; }
        if (n == -1 && errno == EINTR) continue;
        return false;
    }
    return true;
}

static bool readn(int fd, char* data, size_t size)
{
    size_t total = 0;
    while (total < size)
    {
        ssize_t n = recv(fd, data + total, size - total, 0);
        if (n > 0) { total += n; continue; }
        if (n == -1 && errno == EINTR) continue;
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "usage: %s <ip> <port> <count>\n", argv[0]);
        fprintf(stderr, "example: %s 127.0.0.1 5085 10000\n", argv[0]);
        return -1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int count = atoi(argv[3]);
    if (count <= 0) count = 1000;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
    {
        fprintf(stderr, "invalid ip: %s\n", ip);
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "PING");
    uint32_t len = strlen(msg);
    uint32_t netlen = htonl(len);
    std::vector<char> req(4 + len);
    memcpy(req.data(), &netlen, 4);
    memcpy(req.data() + 4, msg, len);

    const auto t0 = std::chrono::steady_clock::now();
    int ok = 0;
    for (int i = 0; i < count; i++)
    {
        if (!writen(sockfd, req.data(), req.size()))
        {
            perror("send failed");
            fprintf(stderr, "failed at request #%d (write)\n", i + 1);
            break;
        }

        if (!readn(sockfd, (char*)&netlen, 4))
        {
            perror("recv header failed");
            fprintf(stderr, "failed at request #%d (read header)\n", i + 1);
            break;
        }

        uint32_t rlen = ntohl(netlen);
        std::vector<char> rsp(rlen);
        if (rlen > 0 && !readn(sockfd, rsp.data(), rlen))
        {
            perror("recv body failed");
            fprintf(stderr, "failed at request #%d (read body, len=%u)\n", i + 1, rlen);
            break;
        }

        ok++;
    }
    const auto t1 = std::chrono::steady_clock::now();
    close(sockfd);

    double elapsed = std::chrono::duration<double>(t1 - t0).count();
    if (elapsed <= 0) elapsed = 0.000001;
    double qps = (elapsed > 0) ? (static_cast<double>(ok) / elapsed) : 0;

    printf("benchmark: ok=%d total=%d elapsed=%.6f qps=%.2f\n", ok, count, elapsed, qps);
    return (ok == count) ? 0 : -1;
}
