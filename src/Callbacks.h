#ifndef LIBUVCPP_CALLBACKS_H
#define LIBUVCPP_CALLBACKS_H

#include "utility.h"
#include <functional>
#include <memory>
NAMESPACE_START

class TcpConnection;

class EventLoop;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::weak_ptr<TcpConnection> WeakTcpConnectionPtr;

typedef std::function<void(TcpConnectionPtr &conn)> ConnectionCallback;

typedef std::function<void(TcpConnectionPtr &conn, char *buff, int len)> MessageCallback;

typedef std::function<void(int errcode, const std::string &errmsg)> ErrorCallback;

typedef std::function<void(TcpConnectionPtr &conn)> WriteCompleteCallback;

typedef std::function<void(const TcpConnectionPtr &conn)> CloseCallback;

typedef std::function<void(EventLoop*)> ThreadInitCallback;

struct Entry
{
    Entry(const WeakTcpConnectionPtr &weakPtr)
            : weakPtr_(weakPtr)
    {
    }

    ~Entry();

    WeakTcpConnectionPtr weakPtr_;
};

typedef std::function<void(std::shared_ptr<Entry>&)> UpdateConnectionCallback;


NAMESPACE_END

#endif //LIBUVCPP_CALLBACKS_H
