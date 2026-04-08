/*
 * EchoServer 类实现
 */
#include "EchoServer.h"
#include "Timestamp.h"
#include <algorithm>
#include <cctype>
#include <cstdio>

EchoServer::EchoServer(const Config& cfg)
    :cfg_(cfg),
     tcpserver_(cfg.ip,cfg.port,cfg.io_threads,cfg.timer_interval,cfg.conn_timeout),
     threadpool_(cfg.work_threads,"WORK")
{
    stats_.setStatsInterval(cfg.stats_interval);

    tcpserver_.setnewconnectioncb(std::bind(&EchoServer::HandleNewConnection,this,std::placeholders::_1));
    tcpserver_.setcloseconnectioncb(std::bind(&EchoServer::HandleClose,this,std::placeholders::_1));
    tcpserver_.seterrorconnectioncb(std::bind(&EchoServer::HandleError,this,std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&EchoServer::HandleMessage,this,std::placeholders::_1,std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&EchoServer::HandleSendComplete,this,std::placeholders::_1));
    tcpserver_.settimeoutcb(std::bind(&EchoServer::HandleTimeOut,this,std::placeholders::_1));
}

EchoServer::~EchoServer()
{
}

void EchoServer::Start()
{
    tcpserver_.start();
}

void EchoServer::Stop()
{
    tcpserver_.stop();
    threadpool_.stop();
    stats_.print();
}

void EchoServer::HandleNewConnection(spConnection conn)
{
    stats_.onConnect();
    printf("client in(fd=%d,ip=%s,port=%d)\n",conn->fd(),conn->ip().c_str(),conn->port());
    fflush(stdout);
}

void EchoServer::HandleClose(spConnection conn)
{
    stats_.onDisconnect();
    printf("client close(fd=%d)\n",conn->fd());
    fflush(stdout);
}

void EchoServer::HandleError(spConnection conn)
{
    stats_.onDisconnect();
    printf("client error(fd=%d)\n",conn->fd());
    fflush(stdout);
}

void EchoServer::HandleMessage(spConnection conn,std::string& message)
{
    stats_.onMessage();
    // 压测期间 epoll 很难超时，用消息流触发统计打印（print 内部有间隔节流）。
    stats_.print();
    threadpool_.addtask(std::bind(&EchoServer::OnMessage,this,conn,message));
}

void EchoServer::HandleSendComplete(spConnection conn)
{
    (void)conn;
}

void EchoServer::HandleTimeOut(EventLoop* loop)
{
    (void)loop;
    if (cfg_.stats_interval > 0)
        stats_.print();
}

void EchoServer::OnMessage(spConnection conn,std::string& message)
{
    if (message=="PING")
    {
        const std::string pong="PONG";
        conn->send(pong.data(),pong.size());
        return;
    }

    if (message=="TIME")
    {
        const std::string now=Timestamp::now().tostring();
        conn->send(now.data(),now.size());
        return;
    }

    const std::string prefix="UPPER ";
    if (message.rfind(prefix,0)==0)
    {
        std::string payload=message.substr(prefix.size());
        std::transform(payload.begin(),payload.end(),payload.begin(),
                       [](unsigned char ch){ return static_cast<char>(std::toupper(ch)); });
        conn->send(payload.data(),payload.size());
        return;
    }

    const std::string reply="[echo]" + message;
    conn->send(reply.data(),reply.size());
}
