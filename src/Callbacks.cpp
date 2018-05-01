#include "Callbacks.h"
#include "TcpConnection.h"

NAMESPACE_START

Entry::~Entry() {
    TcpConnectionPtr conn = weakPtr_.lock();
    if (conn != NULL && conn->idleTimeoutCallback_ != NULL) {
        conn->idleTimeoutCallback_(conn);
    }
}

NAMESPACE_END