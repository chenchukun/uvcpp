#ifndef LIBUVCPP_TCPCONNECTION_H
#define LIBUVCPP_TCPCONNECTION_H

#include "utility.h"
#include "SockAddr.h"
#include "Callbacks.h"
#include <string>
#include <vector>
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

    void setUpdateConnectionCallback(const UpdateConnectionCallback &callback) {
        updateConnectionCallback_ = callback;
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

    void send(const std::string &str);

    void send(const void *ptr, size_t len);

public:
    static void readCallback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

    static void allocCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

private:
    static const size_t BUF_SIZE = 1024;

    void shutdownWrite();

    static void closeCallback(uv_handle_t* handle);

    static void shutdownCallback(uv_shutdown_t* handle, int status=0);

    static void writeCallback(uv_write_t* req, int status);

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

    UpdateConnectionCallback updateConnectionCallback_;

    SockAddr peerAddr_, localAddr_;

private:
    enum BUF_TYPE {BUF_STD_STRING, BUF_VOID_PTR};

    struct WriteContext
    {
        WriteContext(const TcpConnectionPtr &c, const std::string &s)
            : str(move(s)),
              conn(c),
              buffType(BUF_STD_STRING)
        {
            bufs.push_back(uv_buf_t());
            bufs.back().base = const_cast<char*>(str.data());
            bufs.back().len = str.size();
        }

        WriteContext(const TcpConnectionPtr &c, const void *p, size_t len)
            : ptr(p),
              conn(c),
              buffType(BUF_VOID_PTR)
        {
            bufs.push_back(uv_buf_t());
            bufs.back().base = static_cast<char*>(const_cast<void*>(p));
            bufs.back().len = len;
        }

        std::vector<uv_buf_t> bufs;
        BUF_TYPE buffType;
        union {
            const std::string str;
            const void *ptr;
        };
        TcpConnectionPtr conn;
    };
};


NAMESPACE_END

#endif //LIBUVCPP_TCPCONNECTION_H
