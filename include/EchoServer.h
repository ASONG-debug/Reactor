#pragma once
#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "ServerStats.h"
#include "Config.h"

class EchoServer 
{
private:
    Config cfg_;
    TcpServer tcpserver_;
    ThreadPool threadpool_;
    ServerStats stats_;

public:
    EchoServer(const Config& cfg);
    ~EchoServer();

    void Start();
    void Stop();

    void HandleNewConnection(spConnection conn);
    void HandleClose(spConnection conn);
    void HandleError(spConnection conn);
    void HandleMessage(spConnection conn,std::string& message);
    void HandleSendComplete(spConnection conn);
    void HandleTimeOut(EventLoop* loop);

    void OnMessage(spConnection conn,std::string& message);
};