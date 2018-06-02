#include "../TcpServer.h"
#include <iostream>
#include <thread>
using namespace std;
using namespace uvcpp;

int main(int argc, char **argv)
{
    int threadNum = 1;
    if (argc > 2) {
        threadNum = stoi(argv[1]);
    }
    EventLoop eventLoop;
    TcpServer server(&eventLoop);
    server.setThreadNum(threadNum);
/*
    server.setIdleTimeoutCallback(20, [] (TcpConnectionPtr &conn) {
        cout << "Connection [" << conn->getPeerAddr().getIpPort() << "] timeout\n";
        conn->shutdown();
    });
*/

    server.setConnectionCallback([](TcpConnectionPtr &conn) {
        cout << conn->getPeerAddr().getIpPort() << (conn->connected()?" online": " offline") << endl;
    });

    server.setMessageCallback([] (TcpConnectionPtr &conn, Buffer &buffer) {
//        size_t len = buffer.readableBytes();
//        buffer.prepend(to_string(len) + " bytes: ");
	    string str = buffer.retrieveAllAsString();
//        cout << "Recv " << str << endl;
        conn->send(move(str));
    });

/*
    server.setWriteCompleteCallback([] (TcpConnectionPtr &conn) {
//        cout << "Send data to " << conn->getPeerAddr().getIpPort() << " finish" << endl;
    });
*/

    int ret = server.start(SockAddr(6180));
    if (ret != 0) {
        cerr << "Start server failed: " << uv_strerror(ret) << endl;
        exit(1);
    }
    else {
        cout << "Start server success" << endl;
    }

/*
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
*/

    eventLoop.run();
    return 0;
}
