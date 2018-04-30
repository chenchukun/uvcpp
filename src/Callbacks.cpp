#include "Callbacks.h"
#include "TcpConnection.h"

NAMESPACE_START

Entry::~Entry() {
    TcpConnectionPtr conn = weakPtr_.lock();
    if (conn != NULL) {
        conn->shutdown();
    }
}

NAMESPACE_END