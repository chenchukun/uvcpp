#ifndef LIBUVCPP_TCPCONNECTION_H
#define LIBUVCPP_TCPCONNECTION_H

#include "utility.h"
#include "SockAddr.h"
#include <uv.h>
#include <memory>

NAMESPACE_START

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(uv_tcp_t *client) {
        client_ = client;
    }

    ~TcpConnection() {}

    std::shared_ptr<SockAddr> getPeerAddr() const ;

    std::shared_ptr<SockAddr> getLocalAddr() const;

public:
    static void readCallback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

    static void allocCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

private:
    uv_tcp_t *client_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

NAMESPACE_END

#endif //LIBUVCPP_TCPCONNECTION_H
