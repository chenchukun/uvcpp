#include "../TcpServer.h"
#include <iostream>
#include <thread>
using namespace std;
using namespace uvcpp;

int main()
{
    EventLoop eventLoop;
    TcpServer server(&eventLoop);
    server.setThreadNum(4);
    server.setIdleTimeoutCallback(20, [] (TcpConnectionPtr &conn) {
        cout << "Connection [" << conn->getPeerAddr().getIpPort() << "] timeout\n";
        conn->shutdown();
    });

    server.setConnectionCallback([](TcpConnectionPtr &conn) {
        cout << conn->getPeerAddr().getIpPort() << (conn->connected()?" online": " offline") << endl;
    });

    server.setMessageCallback([] (TcpConnectionPtr &conn, char *buff, int len) {
        Buffer buffer(len + 20);
        buffer.append(to_string(len));
        buffer.append(" bytes: ");
        buffer.append(buff, len);
        conn->send(buffer);
        /*
        cout << "Recv " << len << " bytes" << endl;
        if (strncmp(buff, "shutdown", 8) == 0) {
            conn->shutdown();
        }
         */
    });

    server.setWriteCompleteCallback([] (TcpConnectionPtr &conn) {
//j        cout << "Send data to " << conn->getPeerAddr().getIpPort() << " finish" << endl;
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