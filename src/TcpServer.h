#ifndef LIBUVCPP_TCPSERVER_H
#define LIBUVCPP_TCPSERVER_H

#include "utility.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SockAddr.h"
#include <uv.h>
#include <functional>
#include <map>

NAMESPACE_START

class TcpServer
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    typedef void (*ConnectionCallback)(uv_tcp_t *client);

    TcpServer(EventLoop *loop);

    ~TcpServer();

    int start(const SockAddr &addr, int backlog=1024);

    void setThreadNum(size_t num) {
        assert(eventLoopThreadPool_ == NULL);
        assert(num > 0);
        threadNum_ = num;
    }

    void setThreadInitCallback(EventLoopThreadPool::ThreadInitCallback callback) {
        threadInitCallback_ = callback;
    }

    void setConnectionCallback(ConnectionCallback callback) {
        connectionCallback_ = callback;
    }

private:
    static void newConnectionCallback(uv_stream_t* server, int status);

    static void closeCallback(uv_handle_t* handle);

private:
    EventLoop *eventLoop_;

    uv_tcp_t *server_;

    size_t threadNum_;

    EventLoopThreadPool *eventLoopThreadPool_;

    EventLoopThreadPool::ThreadInitCallback threadInitCallback_;

    ConnectionCallback connectionCallback_;
};

NAMESPACE_END

#endif //LIBUVCPP_TCPSERVER_H
