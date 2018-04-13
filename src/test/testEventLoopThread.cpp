#include "../EventLoopThread.h"
#include <iostream>
#include <unistd.h>
using namespace std;
using namespace uvcpp;

void init(EventLoop *loop)
{
    cout << "init thread" << endl;
    loop->runEvery(1000, [] {
        cout << "runEvery" << endl;
    });
}

int main()
{
    EventLoopThread eventLoopThread(init);
    EventLoop *loop = eventLoopThread.startLoop();
    sleep(3);
    return 0;
}