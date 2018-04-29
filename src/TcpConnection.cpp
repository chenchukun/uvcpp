#include "TcpConnection.h"
#include "EventLoop.h"

using namespace std;
NAMESPACE_START


TcpConnection::TcpConnection(EventLoop *loop, uv_tcp_t *client, size_t id)
    : eventLoop_(loop),
    client_(client),
    state_(CONNECTED),
    messageCallback_(NULL),
    errorCallback_(NULL),
    writeCompleteCallback_(NULL),
    closeCallback_(NULL),
    id_(id)
{
    int len = peerAddr_.getAddrLength();
    int ret = uv_tcp_getsockname(client_, localAddr_.getAddr(), &len);
    assert(ret == 0);
    ret = uv_tcp_getpeername(client_, peerAddr_.getAddr(), &len);
    assert(ret == 0);
}

TcpConnection::~TcpConnection()
{
    if (state_ == CLOSE_WAIT) {
        shutdownWrite();
    }
    else {
        state_ = CLOSED;
        uv_tcp_t *client = client_;
        eventLoop_->runInLoopThread([client] {
           uv_close(reinterpret_cast<uv_handle_t*>(client), TcpConnection::closeCallback);
        });
    }
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
        uv_read_stop(stream);
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
    free(handle);
}

void TcpConnection::shutdownCallback(uv_shutdown_t* handle, int status)
{
    free(handle);
    uv_tcp_t *client = static_cast<uv_tcp_t*>(handle->data);
    if (client != NULL) {
        uv_close(reinterpret_cast<uv_handle_t*>(client), TcpConnection::closeCallback);
    }
}

void TcpConnection::shutdownWrite()
{
    state_ = CLOSED;
    uv_shutdown_t *req = static_cast<uv_shutdown_t*>(malloc(sizeof(uv_shutdown_t)));
    uv_tcp_t *client = client_;
    req->data = client;
    eventLoop_->runInLoopThread([client, req] {
        uv_shutdown(req, reinterpret_cast<uv_stream_t*>(client), TcpConnection::shutdownCallback);
    });
}

void TcpConnection::shutdown()
{
    if (state_ == CONNECTED) {
        state_ = FIN_WAIT;
        uv_shutdown_t *req = static_cast<uv_shutdown_t*>(malloc(sizeof(uv_shutdown_t)));
        uv_tcp_t *client = client_;
        req->data = NULL;
        eventLoop_->runInLoopThread([client, req] {
            uv_shutdown(req, reinterpret_cast<uv_stream_t*>(client), TcpConnection::shutdownCallback);
        });
    }
}

void TcpConnection::writeCallback(uv_write_t* req, int status)
{
    WriteContext *context = static_cast<WriteContext*>(req->data);
    if (status == 0) {
        auto &callback = context->conn->writeCompleteCallback_;
        if (callback != NULL) {
            callback(context->conn);
        }
    }
    if (context->buffType == BUF_VOID_PTR) {
        free(const_cast<void*>(context->ptr));
    }
    free(req);
}

void TcpConnection::send(const std::string &str)
{
    uv_write_t *wreq = static_cast<uv_write_t*>(malloc(sizeof(uv_write_t)));
    WriteContext *context = new WriteContext(shared_from_this(), str);
    wreq->data = static_cast<void*>(context);
    uv_stream_t *stream = reinterpret_cast<uv_stream_t*>(client_);
    eventLoop_->runInLoopThread([wreq, context, stream] {
        uv_write(wreq, stream, context->bufs.data(), context->bufs.size(), TcpConnection::writeCallback);
    });
}

void TcpConnection::send(const void *ptr, size_t len)
{
    uv_write_t *wreq = static_cast<uv_write_t*>(malloc(sizeof(uv_write_t)));
    WriteContext *context = new WriteContext(shared_from_this(), ptr, len);
    wreq->data = static_cast<void*>(context);
    uv_stream_t *stream = reinterpret_cast<uv_stream_t*>(client_);
    eventLoop_->runInLoopThread([wreq, context, stream] {
        uv_write(wreq, stream, context->bufs.data(), context->bufs.size(), TcpConnection::writeCallback);
    });
}

NAMESPACE_END