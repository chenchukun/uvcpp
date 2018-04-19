#include "TcpConnection.h"

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

}

void TcpConnection::allocCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{

}

NAMESPACE_END