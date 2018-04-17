#include "../TcpServer.h"
#include <iostream>
#include <thread>
using namespace std;
using namespace uvcpp;

int main()
{
    cout << "main thread: " << this_thread::get_id() << endl;
    EventLoop eventLoop;
    TcpServer server(&eventLoop);
    server.setThreadNum(4);
    server.setConnectionCallback([](uv_tcp_t *client) {
        cout << "connection thread: " << this_thread::get_id() << endl;
    });
    server.start(SockAddr(6180));
    eventLoop.run();
    return 0;
}