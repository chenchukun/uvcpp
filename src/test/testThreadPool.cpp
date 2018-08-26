#include "../ThreadPool.h"
#include <iostream>
using namespace std;
using namespace uvcpp;

int main()
{
    ThreadPool threadPool;
    threadPool.start(4);

    for (int i=0; i<100; ++i) {
        threadPool.run([] {
            cout << this_thread::get_id() << endl;
        });
    }
    threadPool.stop();
    return 0;
}