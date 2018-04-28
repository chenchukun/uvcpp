#include "TcpServer.h"
#include "EventLoop.h"
#include "BlockingQueue.h"

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

bool TcpServer::startRead(uv_tcp_t *client, TcpServer *tcpServer)
{
    int ret = uv_read_start(reinterpret_cast<uv_stream_t*>(client),
                        TcpConnection::allocCallback, TcpConnection::readCallback);
    if (ret != 0) {
        uv_close((uv_handle_t*)client, TcpServer::closeCallback);
        if (tcpServer->errorCallback_ != NULL) {
            tcpServer->errorCallback_(ret, "uv_read_start error");
        }
        return false;
    }
    return true;
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

            if (tcpServer->eventLoopThreadPool_ != NULL) {
                EventLoop *loop = tcpServer->eventLoopThreadPool_->getLoop(id);
                uv_unref(reinterpret_cast<uv_handle_t*>(client));
                ConnectionCallback callback = tcpServer->connectionCallback_;

                loop->runInLoopThread([tcpServer, client, callback, id]{
                    client->loop = EventLoop::getCurrThreadEventLoop()->getLoop();
                    uv_ref(reinterpret_cast<uv_handle_t*>(client));
                    if (!startRead(client, tcpServer)) {
                        return;
                    }
                    TcpConnectionPtr conn = make_shared<TcpConnection>(EventLoop::getCurrThreadEventLoop(), client, id);
                    conn->setConnectionCallback(tcpServer->connectionCallback_);
                    conn->setMessageCallback(tcpServer->messageCallback_);
                    conn->setErrorCallback(tcpServer->errorCallback_);
                    conn->setWriteCompleteCallback(tcpServer->writeCompleteCallback_);
                    conn->setCloseCallback(bind(&TcpServer::removeConnection, tcpServer, std::placeholders::_1));
                    tcpServer->connectionMap_.value()[id] = conn;
                    weak_ptr<TcpConnection> *weakPtr = new weak_ptr<TcpConnection>(conn);
                    client->data = static_cast<void*>(weakPtr);
                    if (callback != NULL) {
                        callback(conn);
                    }
                });
            }
            else {
                if (!startRead(client, tcpServer)) {
                    continue;
                }
                TcpConnectionPtr conn = make_shared<TcpConnection>(EventLoop::getCurrThreadEventLoop(), client, id);
                conn->setConnectionCallback(tcpServer->connectionCallback_);
                conn->setMessageCallback(tcpServer->messageCallback_);
                conn->setErrorCallback(tcpServer->errorCallback_);
                conn->setWriteCompleteCallback(tcpServer->writeCompleteCallback_);
                conn->setCloseCallback(bind(&TcpServer::removeConnection, tcpServer, std::placeholders::_1));
                tcpServer->connectionMap_.value()[id] = conn;
                weak_ptr<TcpConnection> *weakPtr = new weak_ptr<TcpConnection>(conn);
                client->data = static_cast<void*>(weakPtr);
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

map<string, vector<TcpConnectionPtr>> TcpServer::getAllConnection()
{
    BlockingQueue<pair<string, vector<TcpConnectionPtr>>> que;
    size_t num = 0;
    if (eventLoopThreadPool_ == NULL) {
        eventLoop_->runInLoopThread([this, &que] {
            que.put(getConnection("MainLoop", this->connectionMap_.value()));
        });
        num = 1;
    }
    else {
        vector<EventLoop*> loops = eventLoopThreadPool_->getAllLoops();
        for (size_t i=0; i<loops.size(); ++i) {
            loops[i]->runInLoopThread([this, &que, i] {
                que.put(getConnection("ChildLoop_" + to_string(i), this->connectionMap_.value()));
            });
        }
        num = loops.size();
    }
    map<string, vector<TcpConnectionPtr>> results;
    for (size_t i=0; i<num; ++i) {
        auto value = que.take();
        results[value.first] = value.second;
    }
    return results;
}

pair<string, vector<TcpConnectionPtr>> TcpServer::getConnection(const string name, const map<size_t, TcpConnectionPtr> &cmap)
{
    vector<TcpConnectionPtr> conns;
    map<size_t, TcpConnectionPtr>::const_iterator it = cmap.cbegin();
    while (it != cmap.cend()) {
        conns.push_back(it->second);
        ++it;
    }
    return make_pair(name, conns);
}

NAMESPACE_END