#include "../TcpClient.h"
#include <iostream>
using namespace std;
using namespace uvcpp;

int main(int argc, char **argv)
{
    string ip = "127.0.0.1";
    int port = 6180;
    if (argc == 3) {
        ip = argv[1];
        port = stoi(argv[2]);
    }
    else if (argc == 2) {
        ip = argv[1];
    }
    EventLoop eventLoop;

    TcpClient client(&eventLoop, SockAddr(ip.c_str(), port));
    client.setConnectionCallback([] (TcpConnectionPtr &conn) {
        if (conn->connected()) {
            cout << "Connect to " << conn->getPeerAddr().getIpPort() << " success" << endl;
            thread t([conn] {
                string str;
                while (getline(cin, str)) {
                    conn->send(str);
                    if (str == "shutdown") {
                        conn->shutdown();
                    }
                }
            });
            t.detach();
        }
        else {
            cout << "disconnect" << endl;
        }
    });
    client.setMessageCallback([] (TcpConnectionPtr &conn, char *buff, int len) {
        buff[len] = 0;
        cout << buff << endl;
    });
    client.connect();
    eventLoop.run();
    return 0;
}