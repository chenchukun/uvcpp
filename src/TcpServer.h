#ifndef LIBUVCPP_TCPSERVER_H
#define LIBUVCPP_TCPSERVER_H

#include "utility.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SockAddr.h"
#include "TcpConnection.h"
#include "ThreadLocal.h"
#include "Callbacks.h"
#include <uv.h>
#include <functional>
#include <map>
#include <unordered_set>
#include <queue>

NAMESPACE_START

class TcpServer
{
public:

    TcpServer(EventLoop *loop);

    ~TcpServer();

    TcpServer(const TcpServer&) = delete;

    TcpServer& operator=(const TcpServer&) = delete;

    int start(const SockAddr &addr, int backlog=1024);

    void setThreadNum(size_t num) {
        assert(eventLoopThreadPool_ == NULL);
        assert(num > 0);
        threadNum_ = num;
    }

    void setThreadInitCallback(const EventLoopThreadPool::ThreadInitCallback &callback) {
        threadInitCallback_ = callback;
    }

    void setConnectionCallback(const ConnectionCallback &callback) {
        connectionCallback_ = callback;
    }

    void setMessageCallback(const MessageCallback &callback) {
        messageCallback_ = callback;
    }

    void setErrorCallback(const ErrorCallback &callback) {
        errorCallback_ = callback;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &callback) {
        writeCompleteCallback_ = callback;
    }

    void setIdleTimeoutCallback(int second, const IdleTimeoutCallback &callback) {
        timeout_ = second;
        idleTimeoutCallback_ = callback;
    }

    // debug
    std::map<std::string, std::vector<TcpConnectionPtr>> getAllConnection();

private:
    static void newConnectionCallback(uv_stream_t* server, int status);

    static void closeCallback(uv_handle_t* handle);

    static bool startRead(uv_tcp_t *client, TcpServer *tcpServer);

    void removeConnection(const TcpConnectionPtr &conn) {
        size_t id = conn->id();
        connectionMap_.value().erase(id);
    }

    void updateConnection(std::shared_ptr<Entry> &entryPtr) {
        whell_.value().back().insert(entryPtr);
    }

    std::pair<std::string, std::vector<TcpConnectionPtr>> getConnection(const std::string name, const std::map<size_t, TcpConnectionPtr> &cmap);

    void threadInit(EventLoop *eventLoop);

    void checkConnection();

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

    IdleTimeoutCallback idleTimeoutCallback_;

    ThreadLocal<std::map<size_t, TcpConnectionPtr>> connectionMap_;

    static const size_t ACCEPT_MAX = 10;

    int timeout_;

    ThreadLocal<std::queue<std::unordered_set<std::shared_ptr<Entry>>>> whell_;
};

NAMESPACE_END

#endif //LIBUVCPP_TCPSERVER_H
