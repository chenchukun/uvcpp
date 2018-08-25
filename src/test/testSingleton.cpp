#include "../Singleton.h"
#include "../ThreadLocal.h"
#include <iostream>
#include <thread>

using namespace std;
using namespace uvcpp;

class Foo
{
public:
    Foo() {
        cout << "call Foo(): " << this_thread::get_id() << endl;
    }

    ~Foo() {
        cout << "call ~Foo()" << endl;
    }

    void fun()
    {
        cout << "call fun()" << endl;
    }
};

int main()
{
    Singleton<Foo>::instance().fun();
    Singleton<Foo>::instance().fun();

    Singleton<ThreadLocal<Foo>>::instance().value().fun();

    Singleton<ThreadLocal<Foo>>::instance().value().fun();

    thread t([] {
        Singleton<ThreadLocal<Foo>>::instance().value().fun();
        Singleton<ThreadLocal<Foo>>::instance().value().fun();
    });
    t.join();

    return 0;
}