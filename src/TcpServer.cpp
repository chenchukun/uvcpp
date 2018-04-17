#include "TcpServer.h"
#include "EventLoop.h"

NAMESPACE_START

TcpServer::TcpServer(EventLoop *loop)
    : eventLoop_(loop),
      server_(NULL),
      eventLoopThreadPool_(NULL),
      threadNum_(0),
      threadInitCallback_(NULL),
      connectionCallback_(NULL),
      connectionId_(0),
      messageCallback_(NULL),
      errorCallback_(NULL),
      writeCompleteCallback_(NULL)
{

}

TcpServer::~TcpServer()
{
    uv_close(reinterpret_cast<uv_handle_t*>(server_), NULL);
    free(server_);
    if (eventLoopThreadPool_ != NULL) {
        delete eventLoopThreadPool_;
    }
}

int TcpServer::start(const SockAddr &addr, int backlog)
{
    if (threadNum_ > 0) {
        eventLoopThreadPool_ = new EventLoopThreadPool(threadNum_);
        eventLoopThreadPool_->start(threadInitCallback_);
    }
    server_ = static_cast<uv_tcp_t*>(malloc(sizeof(uv_tcp_t)));

    server_->data = this;

    CHECK_ZERO_RETURN(uv_tcp_init(eventLoop_->getLoop(), server_));

    CHECK_ZERO_RETURN(uv_tcp_bind(server_, addr.getAddr(), 0));

    CHECK_ZERO_RETURN(uv_listen(reinterpret_cast<uv_stream_t*>(server_), backlog, TcpServer::newConnectionCallback));

    return 0;
}

void TcpServer::closeCallback(uv_handle_t* handle)
{
    free(handle);
}

void TcpServer::newConnectionCallback(uv_stream_t* server, int status)
{
    if (status != 0) {
        LOG_ERROR("newConnectionCallback: %s(%s)", uv_strerror(status), uv_err_name(status));
        return;
    }
    TcpServer *tcpServer = static_cast<TcpServer*>(server->data);
    for (int i=0; i<10; ++i) {
        uv_tcp_t *client = static_cast<uv_tcp_t*>(malloc(sizeof(uv_tcp_t)));
        uv_tcp_init(tcpServer->eventLoop_->getLoop(), client);
        int ret = uv_accept(server, reinterpret_cast<uv_stream_t*>(client));
        if (ret == 0) {
            if (tcpServer->eventLoopThreadPool_ != NULL) {
                size_t id = tcpServer->connectionId_;
                tcpServer->connectionId_++;
                EventLoop *loop = tcpServer->eventLoopThreadPool_->getLoop(id);
                uv_unref(reinterpret_cast<uv_handle_t*>(client));
                ConnectionCallback callback = tcpServer->connectionCallback_;
                loop->runInLoopThread([client, callback, id]{
                    client->loop = EventLoop::getCurrThreadEventLoop()->getLoop();
                    uv_ref(reinterpret_cast<uv_handle_t*>(client));
                    // call connectionCallback
                    if (callback != NULL) {
                        callback(client);
                    }
                });
            }
            else {
                // call connectionCallback
                if (tcpServer->connectionCallback_ != NULL) {
                    tcpServer->connectionCallback_(client);
                }
            }
        }
        else if (ret == UV_EAGAIN) {
            uv_close((uv_handle_t*)client, TcpServer::closeCallback);
            break;
        }
        else {
            LOG_ERROR("uv_accept: %s(%s)", uv_strerror(ret), uv_err_name(ret));
            if (tcpServer->errorCallback_ != NULL) {
                tcpServer->errorCallback_(NULL, ret, "uv_accept error");
            }
            free(client);
        }
    }
}

NAMESPACE_END