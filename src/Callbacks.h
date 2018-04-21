#ifndef LIBUVCPP_CALLBACKS_H
#define LIBUVCPP_CALLBACKS_H

#include "utility.h"
#include <functional>
NAMESPACE_START

class TcpConnection;

class EventLoop;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef void (*ConnectionCallback)(TcpConnectionPtr &conn);

typedef void (*MessageCallback)(TcpConnectionPtr &conn, char *buff, int len);

typedef void (*ErrorCallback)(int errcode, const std::string &errmsg);

typedef void (*WriteCompleteCallback)(TcpConnectionPtr &conn);

typedef void (*CloseCallback)(const TcpConnectionPtr &conn);

typedef std::function<void(EventLoop*)> ThreadInitCallback;

NAMESPACE_END

#endif //LIBUVCPP_CALLBACKS_H
