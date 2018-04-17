#include "../ThreadLocal.h"
#include <thread>
#include <sstream>
#include <iostream>
#include <vector>
#include <mutex>
using namespace std;
using namespace uvcpp;

class Test
{
public:
    Test(size_t num) : num_(num) {

    }

    ~Test() {
        for (auto t : threads_) {
            t->join();
            delete t;
        }
    }


    void start() {
        for (size_t i=0; i<num_; ++i) {
            thread *t = new thread([this, i] {
                this->mutex_.lock();
                cout << this_thread::get_id() << " " << &(this->strings.value()) << endl;
                this->mutex_.unlock();
                stringstream stream;
                stream << i;
                this->strings.value() = stream.str();
                this->mutex_.lock();
                cout << this_thread::get_id() << " " << this->strings.value() << endl;
                this->mutex_.unlock();
            });
            threads_.push_back(t);
        }
    }

private:
    vector<thread*> threads_;

    size_t num_;

    ThreadLocal<string> strings;

    mutex mutex_;
};

int main()
{
    Test test(4);
    test.start();
    return 0;
}