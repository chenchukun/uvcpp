//
// Created by chenchukun on 18/4/1.
//
#include "EventLoop.h"
#include <iostream>
using namespace uvcpp;
using namespace std;

int main()
{
    EventLoop loop;
    loop.runEvery(1000, [] {
        cout << "call runEvery callback" << endl;
    });
    loop.run();
    return 0;
}
