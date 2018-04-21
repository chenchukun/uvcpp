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
    typedef enum {CONNECTED, FIN_WAIT, CLOSE_WAIT, CLOSED} ConnectionState;

    TcpConnection(EventLoop *loop, uv_tcp_t *client, size_t id)
        : eventLoop_(loop),
          client_(client),
          state_(CONNECTED),
          messageCallback_(NULL),
          errorCallback_(NULL),
          writeCompleteCallback_(NULL),
          closeCallback_(NULL),
          id_(id)
    {

    }

    ~TcpConnection() {}

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

    void setCloseCallback(CloseCallback callback) {
        closeCallback_ = callback;
    }

    size_t id() const {
        return id_;
    }

    std::shared_ptr<SockAddr> getPeerAddr() const ;

    std::shared_ptr<SockAddr> getLocalAddr() const;

    ConnectionState getConnectionState() const {
        return state_;
    }

    bool connected() const {
        return state_ == CONNECTED;
    }

    void send(std::string &&str);

public:
    static void readCallback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

    static void allocCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

    static void closeCallback(uv_handle_t* handle);

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
};


NAMESPACE_END

#endif //LIBUVCPP_TCPCONNECTION_H
