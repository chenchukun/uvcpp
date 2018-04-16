#include "../TcpServer.h"
#include <iostream>
using namespace std;
using namespace uvcpp;

int main()
{
    EventLoop eventLoop;
    TcpServer server(&eventLoop);
    server.setConnectionCallback([](uv_tcp_t *client) {
        cout << "new connection" << endl;
    });
    server.start(SockAddr(6180));
    eventLoop.run();
    return 0;
}