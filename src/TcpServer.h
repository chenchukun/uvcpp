#ifndef LIBUVCPP_TCPSERVER_H
#define LIBUVCPP_TCPSERVER_H

#include "utility.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SockAddr.h"
#include "TcpConnection.h"
#include "ThreadLocal.h"
#include <uv.h>
#include <functional>
#include <map>

NAMESPACE_START

class TcpServer
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    typedef void (*ConnectionCallback)(TcpConnectionPtr &conn);

    typedef void (*MessageCallback)(TcpConnectionPtr &conn);

    typedef void (*ErrorCallback)(int errcode, const std::string &errmsg);

    typedef void (*WriteCompleteCallback)(TcpConnectionPtr &conn);

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

    void setMessageCallback(MessageCallback callback) {
        messageCallback_ = callback;
    }

    void setErrorCallback(ErrorCallback callback) {
        errorCallback_ = callback;
    }

    void setWriteCompleteCallback(WriteCompleteCallback callback) {
        writeCompleteCallback_ = callback;
    }

private:
    static void newConnectionCallback(uv_stream_t* server, int status);

    static void closeCallback(uv_handle_t* handle);

private:
    size_t connectionId_;

    EventLoop *eventLoop_;

    uv_tcp_t *server_;

    size_t threadNum_;

    EventLoopThreadPool *eventLoopThreadPool_;

    EventLoopThreadPool::ThreadInitCallback threadInitCallback_;

    ConnectionCallback connectionCallback_;

    MessageCallback messageCallback_;

    ErrorCallback errorCallback_;

    WriteCompleteCallback writeCompleteCallback_;

    ThreadLocal<std::map<size_t, TcpConnectionPtr>> connectionMap_;

    static const size_t ACCEPT_MAX = 10;
};

NAMESPACE_END

#endif //LIBUVCPP_TCPSERVER_H
