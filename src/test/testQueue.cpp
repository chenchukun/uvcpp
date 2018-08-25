#include "../BlockingQueue.h"
#include "../BoundedBlockingQueue.h"
#include <iostream>
#include <thread>
using namespace std;
using namespace uvcpp;

BlockingQueue<int> que;

BoundedBlockingQueue<int> bque(10);

void testBlockingQueue()
{
    thread consumer([] {
        while (true) {
            int value;
            bool ret = que.poll(value, 1000);
            if (ret) {
                cout << "consumer: " << value << endl;
            }
            else {
                cout << "timeout" << endl;
            }
        }
    });
    thread producer([] {
        int value;
        while (cin >> value) {
            que.put(value);
        }
    });
    producer.join();
    consumer.detach();
}

void testBoundedBlockingQueue()
{
    thread consumer([] {
        while (true) {
            int value = bque.take();
            cout << "consumer: " << value << endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    thread producer([] {
        for (int i=0; i<100; ++i) {
            bque.put(i);
            cout << "producer: put " << i << endl;
        }
    });
    producer.join();
    consumer.detach();
}

int main()
{
    testBlockingQueue();
    testBoundedBlockingQueue();
    return 0;
}