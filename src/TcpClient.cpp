#include "TcpClient.h"

using namespace std;

NAMESPACE_START

void TcpClient::connectImpl()
{
    uv_connect_t *req = static_cast<uv_connect_t*>(malloc(sizeof(uv_connect_t)));
    req->data = static_cast<void*>(this);
    client_ = static_cast<uv_tcp_t*>(malloc(sizeof(uv_tcp_t)));
    uv_tcp_init(eventLoop_->getLoop(), client_);
    uv_tcp_t *client = client_;
    SockAddr addr = addr_;
    eventLoop_->runInLoopThread([req, client, addr] () {
        uv_tcp_connect(req, client, addr.getAddr(), TcpClient::connectionCallback);
    });
}

void TcpClient::connectionCallback(uv_connect_t *req, int status)
{
    TcpClient *client = static_cast<TcpClient*>(req->data);
    if (status == 0) {
        int ret = uv_read_start(reinterpret_cast<uv_stream_t*>(client->client_),
                                TcpConnection::allocCallback, TcpConnection::readCallback);
        if (ret != 0) {
            uv_close((uv_handle_t*)client, TcpClient::closeCallback);
            return;
        }

        client->connectionPtr_ = make_shared<TcpConnection>(client->eventLoop_, client->client_);
        WeakTcpConnectionPtr weakPtr(client->connectionPtr_);
        auto *data = new pair<weak_ptr<TcpConnection>, weak_ptr<Entry>>(weakPtr, weak_ptr<Entry>(shared_ptr<Entry>()));
        client->client_->data = static_cast<void*>(data);
        client->connectionPtr_->setConnectionCallback(client->connectionCallback_);
        client->connectionPtr_->setMessageCallback(client->messageCallback_);
        client->connectionPtr_->setWriteCompleteCallback(client->writeCompleteCallback_);
        if (client->connectionCallback_ != NULL) {
            client->connectionCallback_(client->connectionPtr_);
        }
        free(req);
    }
    else {
        // ERROR LOG
        LOG_ERROR("Connection to %s fail: %s", client->addr_.getIpPort().c_str(), uv_strerror(status));
        uv_close(reinterpret_cast<uv_handle_t*>(client->client_), TcpClient::closeCallback);
        if (++client->curr_ <= client->retryNum_) {
            uint64_t timeout = client->curr_ * 10000;
            client->eventLoop_->runAfter(timeout, [client] () {
                client->connectImpl();
            });
        }
    }
}

NAMESPACE_END