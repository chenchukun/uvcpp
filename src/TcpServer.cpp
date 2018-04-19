#include "TcpServer.h"
#include "EventLoop.h"

using namespace std;

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
    TcpServer *tcpServer = static_cast<TcpServer*>(server->data);

    if (status != 0) {
        LOG_ERROR("newConnectionCallback: %s(%s)", uv_strerror(status), uv_err_name(status));
        if (tcpServer->errorCallback_ != NULL) {
            tcpServer->errorCallback_(status, "uv_listen error");
        }
        return;
    }
    for (size_t i=0; i<ACCEPT_MAX; ++i) {
        uv_tcp_t *client = static_cast<uv_tcp_t*>(malloc(sizeof(uv_tcp_t)));
        uv_tcp_init(tcpServer->eventLoop_->getLoop(), client);

        int ret = uv_accept(server, reinterpret_cast<uv_stream_t*>(client));
        if (ret == 0) {
            size_t id = tcpServer->connectionId_;
            tcpServer->connectionId_++;

            ret = uv_read_start(reinterpret_cast<uv_stream_t*>(client),
                TcpConnection::allocCallback, TcpConnection::readCallback);
            if (ret != 0) {
                uv_close((uv_handle_t*)client, TcpServer::closeCallback);
                LOG_ERROR("uv_read_start: %s(%s)", uv_strerror(ret), uv_err_name(ret));
                if (tcpServer->errorCallback_ != NULL) {
                    tcpServer->errorCallback_(ret, "uv_read_start error");
                }
                continue;
            }

            if (tcpServer->eventLoopThreadPool_ != NULL) {
                EventLoop *loop = tcpServer->eventLoopThreadPool_->getLoop(id);
                uv_unref(reinterpret_cast<uv_handle_t*>(client));
                ConnectionCallback callback = tcpServer->connectionCallback_;

                loop->runInLoopThread([tcpServer, client, callback, id]{
                    client->loop = EventLoop::getCurrThreadEventLoop()->getLoop();
                    uv_ref(reinterpret_cast<uv_handle_t*>(client));
                    TcpConnectionPtr conn = make_shared<TcpConnection>(client);
                    tcpServer->connectionMap_.value()[id] = conn;

                    if (callback != NULL) {
                        callback(conn);
                    }
                });
            }
            else {
                TcpConnectionPtr conn = make_shared<TcpConnection>(client);

                if (tcpServer->connectionCallback_ != NULL) {
                    tcpServer->connectionCallback_(conn);
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
                tcpServer->errorCallback_(ret, "uv_accept error");
            }
            free(client);
        }
    }
}

NAMESPACE_END