#ifndef LIBUVCPP_TCPCLIENT_H
#define LIBUVCPP_TCPCLIENT_H

#include "utility.h"
#include "EventLoop.h"
#include "SockAddr.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include <uv.h>

NAMESPACE_START

class TcpClient
{
public:
    TcpClient(EventLoop *eventLoop, const SockAddr &addr)
        : eventLoop_(eventLoop),
          client_(NULL),
          connectionCallback_(NULL),
          messageCallback_(NULL),
          writeCompleteCallback_(NULL),
          connectionPtr_(NULL),
          retryNum_(3),
          curr_(0),
          addr_(addr)
    {

    }

    ~TcpClient() {
        connectionPtr_->shutdown();
    }

    void setConnectionCallback(const ConnectionCallback &callback) {
        connectionCallback_ = callback;
    }

    void setMessageCallback(const MessageCallback &callback) {
        messageCallback_ = callback;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &callback) {
        writeCompleteCallback_ = callback;
    }

    void connect() {
        curr_ = 0;
        connectImpl();
    }

    void disconnect();

    void setRetryNum(size_t num) {
        retryNum_ = num;
    }

    size_t getRetryNum() {
        return retryNum_;
    }

    EventLoop* getEventLoop() {
        return eventLoop_;
    }

    TcpConnectionPtr getConnection() const {
        return connectionPtr_;
    }

private:
    static void connectionCallback(uv_connect_t *req, int status);

    static void closeCallback(uv_handle_t* handle) {
        free(handle);
    }

    void connectImpl();

private:
    EventLoop *eventLoop_;

    uv_tcp_t *client_;

    TcpConnectionPtr connectionPtr_;

    ConnectionCallback connectionCallback_;

    MessageCallback messageCallback_;

    WriteCompleteCallback writeCompleteCallback_;

    size_t retryNum_;

    size_t curr_;

    SockAddr addr_;
};

NAMESPACE_END

#endif //LIBUVCPP_TCPCLIENT_H
