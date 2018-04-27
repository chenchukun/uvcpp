#include "TcpConnection.h"
#include "EventLoop.h"

using namespace std;
NAMESPACE_START

TcpConnection::~TcpConnection()
{
    if (state_ == CLOSE_WAIT) {
        shutdownWrite();
    }
}

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
        // 更新连接状态
        conn->state_ = conn->state_==CONNECTED? CLOSE_WAIT: CLOSED;
        conn->connectionCallback_(conn);
        // 调用closeCallback,从TcpServer中删除该连接,这里不需要调用runInThread
        // 若此时用户不拥有该连接,则连接将析构,
        // 否则可以继续发送数据直到调用shutdown()
        conn->closeCallback_(conn);
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

void TcpConnection::shutdownCallback(uv_shutdown_t* handle, int status)
{
    weak_ptr<TcpConnection> *weak = static_cast<weak_ptr<TcpConnection>*>(handle->data);
    if (status != 0) {
        TcpConnectionPtr conn = weak->lock();
        if (conn) {
            conn->errorCallback_(status, uv_strerror(status));
        }
    }
    delete weak;
    free(handle);
}

void TcpConnection::shutdownWrite()
{
    state_ = state_ == CONNECTED? FIN_WAIT: CLOSED;
    uv_shutdown_t *req = static_cast<uv_shutdown_t*>(malloc(sizeof(uv_shutdown_t)));
    req->data = static_cast<void*>(new weak_ptr<TcpConnection>(shared_from_this()));
    uv_tcp_t *client = client_;
    eventLoop_->runInLoopThread([client, req] {
        uv_shutdown(req, reinterpret_cast<uv_stream_t*>(client), TcpConnection::shutdownCallback);
    });
}

void TcpConnection::shutdown()
{
    if (state_ != FIN_WAIT && state_ != CLOSED) {
        shutdownWrite();
    }
}

NAMESPACE_END