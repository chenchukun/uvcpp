#include "TcpConnection.h"
#include "EventLoop.h"

using namespace std;
NAMESPACE_START

shared_ptr<SockAddr> TcpConnection::getLocalAddr() const
{
    std::shared_ptr<SockAddr> paddr = make_shared<SockAddr>();
    int len = paddr->getAddrLength();
    int ret = uv_tcp_getsockname(client_, paddr->getAddr(), &len);
    if (ret != 0) {
        return NULL;
    }
    return paddr;
}

shared_ptr<SockAddr> TcpConnection::getPeerAddr() const
{
    std::shared_ptr<SockAddr> paddr = make_shared<SockAddr>();
    int len = paddr->getAddrLength();
    int ret = uv_tcp_getpeername(client_, paddr->getAddr(), &len);
    if (ret != 0) {
        return NULL;
    }
    return paddr;
}

void TcpConnection::readCallback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    TcpConnectionPtr conn = static_cast<weak_ptr<TcpConnection>*>(stream->data)->lock();
    assert(conn != NULL);
    // 对端关闭连接
    if (nread == UV_EOF) {
        assert(conn->state_ != CLOSED && conn->state_ != CLOSE_WAIT);
        if (conn->state_ == CONNECTED) {
            conn->state_ = CLOSE_WAIT;
            conn->connectionCallback_(conn);
            uv_close(reinterpret_cast<uv_handle_t*>(conn->client_), TcpConnection::closeCallback);
        }
        else {
            conn->state_ = CLOSED;
            conn->connectionCallback_(conn);
            uv_close(reinterpret_cast<uv_handle_t*>(conn->client_), TcpConnection::closeCallback);
        }
    }
    else if (nread > 0){
        conn->messageCallback_(conn, conn->buff_, nread);
    }
    else {
        conn->errorCallback_(nread, uv_strerror(nread));
    }
}

void TcpConnection::allocCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    auto *connWeakPtr = static_cast<weak_ptr<TcpConnection>*>(handle->data);
    TcpConnectionPtr conn = connWeakPtr->lock();
    if (conn) {
        buf->base = conn->buff_;
        buf->len = TcpConnection::BUF_SIZE;
    }
    else {
        buf->base = NULL;
        buf->len = 0;
    }
}

void TcpConnection::closeCallback(uv_handle_t* handle)
{
    weak_ptr<TcpConnection> *weak = static_cast<weak_ptr<TcpConnection>*>(handle->data);
    TcpConnectionPtr conn = weak->lock();
    assert(conn != NULL);
    conn->eventLoop_->runInLoopThread([conn] {
        conn->closeCallback_(conn);
    });
    delete weak;
    free(handle);
}

NAMESPACE_END