#ifndef LIBUVCPP_TCPCONNECTION_H
#define LIBUVCPP_TCPCONNECTION_H

#include "utility.h"
#include "SockAddr.h"
#include "Callbacks.h"
#include <string>
#include <uv.h>
#include <memory>

NAMESPACE_START

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    // 连接状态、主动关闭发送了FIN、被动关闭接收到FIN、关闭
    typedef enum {CONNECTED, FIN_WAIT, CLOSE_WAIT, CLOSED} ConnectionState;

    TcpConnection(EventLoop *loop, uv_tcp_t *client, size_t id);

    ~TcpConnection();

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

    void setCloseCallback(const CloseCallback &callback) {
        closeCallback_ = callback;
    }

    size_t id() const {
        return id_;
    }

    const SockAddr& getPeerAddr() const {
        return peerAddr_;
    }

    const SockAddr& getLocalAddr() const {
        return localAddr_;
    }

    ConnectionState getConnectionState() const {
        return state_;
    }

    bool connected() const {
        return state_ == CONNECTED;
    }

    void shutdown();

    void send(std::string &&str);

public:
    static void readCallback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

    static void allocCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

private:
    enum BUF_TYPE {BUF_STD_STRING};

    struct WriteContext {
        uv_buf_t *buf;
        BUF_TYPE buffType;
        union {
            std::string str;
        };
        std::shared_ptr<TcpConnection> conn;
    };

    static const size_t BUF_SIZE = 1024;

    void shutdownWrite();

    static void closeCallback(uv_handle_t* handle);

    static void shutdownCallback(uv_shutdown_t* handle, int status=0);

private:
    uv_tcp_t *client_;

    EventLoop *eventLoop_;

    size_t id_;

    ConnectionState state_;

    char buff_[BUF_SIZE];

    ConnectionCallback connectionCallback_;

    MessageCallback messageCallback_;

    ErrorCallback errorCallback_;

    WriteCompleteCallback writeCompleteCallback_;

    CloseCallback closeCallback_;

    SockAddr peerAddr_, localAddr_;
};


NAMESPACE_END

#endif //LIBUVCPP_TCPCONNECTION_H
