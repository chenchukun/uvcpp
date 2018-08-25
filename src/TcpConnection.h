#ifndef LIBUVCPP_TCPCONNECTION_H
#define LIBUVCPP_TCPCONNECTION_H

#include "utility.h"
#include "SockAddr.h"
#include "Callbacks.h"
#include "Buffer.h"
#include <string>
#include <vector>
#include <uv.h>
#include <memory>

NAMESPACE_START

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    friend class Entry;

    // 连接状态、主动关闭发送了FIN、被动关闭接收到FIN、关闭
    typedef enum {CONNECTED, FIN_WAIT, CLOSE_WAIT, CLOSED} ConnectionState;

    TcpConnection(EventLoop *loop, uv_tcp_t *client, size_t id=-1);

    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;

    TcpConnection& operator=(const TcpConnection&) = delete;

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

    void setIdleTimeoutCallback(const IdleTimeoutCallback &callback) {
        idleTimeoutCallback_ = callback;
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

    void send(std::string &&str);

    void send(const void *ptr, size_t len);

    void send(Buffer &buffer);

public:
    static void readCallback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

    static void allocCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

private:
    static const size_t BUF_SIZE = 4096;

    void shutdownWrite();

    static void closeCallback(uv_handle_t* handle);

    static void shutdownCallback(uv_shutdown_t* handle, int status=0);

    static void writeCallback(uv_write_t* req, int status);

private:
    uv_tcp_t *client_;

    EventLoop *eventLoop_;

    size_t id_;

    ConnectionState state_;

    Buffer buffer_;

    ConnectionCallback connectionCallback_;

    MessageCallback messageCallback_;

    ErrorCallback errorCallback_;

    WriteCompleteCallback writeCompleteCallback_;

    CloseCallback closeCallback_;

    UpdateConnectionCallback updateConnectionCallback_;

    IdleTimeoutCallback idleTimeoutCallback_;

    SockAddr peerAddr_, localAddr_;

private:
    enum BUF_TYPE {BUF_STD_STRING, BUF_VOID_PTR, BUF_BUFFER};

    struct WriteContext
    {
        WriteContext(const TcpConnectionPtr &c, const std::string &s)
            : buffType(BUF_STD_STRING),
              str(s),
              conn(c)
        {
            bufs.push_back(uv_buf_t());
            bufs.back().base = const_cast<char*>(str.data());
            bufs.back().len = str.size();
        }

        WriteContext(const TcpConnectionPtr &c, std::string &&s)
            : buffType(BUF_STD_STRING),
              str(std::move(s)),
              conn(c)
        {
            bufs.push_back(uv_buf_t());
            bufs.back().base = const_cast<char*>(str.data());
            bufs.back().len = str.size();
        }

        WriteContext(const TcpConnectionPtr &c, const void *p, size_t len)
            : buffType(BUF_VOID_PTR),
              ptr(p),
              conn(c)
        {
            bufs.push_back(uv_buf_t());
            bufs.back().base = const_cast<char*>(static_cast<const char*>(ptr));
            bufs.back().len = len;
        }

        WriteContext(const TcpConnectionPtr &c, Buffer &buf)
            : buffType(BUF_BUFFER),
              buffer(std::move(buf)),
              conn(c)
        {
            std::vector<std::pair<char*, size_t> > pairBufs;
            buffer.all(pairBufs);
            for (int i=0; i<pairBufs.size(); ++i) {
                bufs.push_back(uv_buf_t());
                bufs.back().base = pairBufs[i].first;
                bufs.back().len = pairBufs[i].second;
            }
        }

        ~WriteContext() {
            if (buffType == BUF_VOID_PTR) {
                free(const_cast<void*>(ptr));
            }
        }

        std::vector<uv_buf_t> bufs;
        BUF_TYPE buffType;
        const std::string str;
        const void *ptr;
        Buffer buffer;
        TcpConnectionPtr conn;
    };
};


NAMESPACE_END

#endif //LIBUVCPP_TCPCONNECTION_H
