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
        cout << conn->getPeerAddr().getIpPort() << (conn->connected()?" online": " offline") << endl;
    });
    server.setMessageCallback([] (TcpConnectionPtr &conn, char *buff, int len) {
        buff[len] = 0;
        cout << "Recv: " << buff << endl;
        if (strncmp(buff, "shutdown", 8) == 0) {
            conn->shutdown();
        }
    });
    int ret = server.start(SockAddr(6180));
    if (ret != 0) {
        cerr << "Start server failed: " << uv_strerror(ret) << endl;
        exit(1);
    }
    else {
        cout << "Start server success" << endl;
    }
    eventLoop.runEvery(10000, [&server] {
        auto connMap = server.getAllConnection();
        auto it = connMap.begin();
        while (it != connMap.end()) {
            cout << it->first << ":\n";
            auto conns = it->second;
            if (conns.size() == 0) {
                cout << "\tConnection is empty" << endl;
                ++it;
                continue;
            }
            for (size_t i=0; i<conns.size(); ++i) {
                TcpConnectionPtr conn = conns[i];
                auto laddr = conn->getLocalAddr();
                auto raddr = conn->getPeerAddr();
                cout << "\t(" << laddr.getIpPort() << ", " << raddr.getIpPort() << ")\n";
            }
            cout << endl;
            ++it;
        }
        cout << "-----------------------------------------" << endl;
    });
    eventLoop.run();
    return 0;
}