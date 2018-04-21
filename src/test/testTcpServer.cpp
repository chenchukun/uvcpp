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
    server.setConnectionCallback([](TcpConnectionPtr &conn) {
        cout << "Connection thread: " << this_thread::get_id() << endl;
        cout << conn->getPeerAddr()->getIpPort() << (conn->connected()?" online": " offline") << endl;

    });
    server.setMessageCallback([] (TcpConnectionPtr &conn, char *buff, int len) {
        buff[len] = 0;
        cout << "Recv: " << buff << endl;
    });
    server.start(SockAddr(6180));
    eventLoop.run();
    return 0;
}