//
// Created by chenchukun on 18/4/3.
//
#include "../Timeval.h"
#include <iostream>
using namespace std;
using namespace uvcpp;

int main()
{
    Timeval now;
    cout << "now.second() = " << now.second() << endl;
    cout << "now.millisecond() = " << now.millisecond() << endl;
    cout << "now.microsecond() = " << now.microsecond() << endl;
    Timeval t(1234567);
    if (now > t) {
        cout << now.microsecond() << " > " << t.microsecond() << endl;
    }
    if (now >= t) {
        cout << now.microsecond() << " >= " << t.microsecond() << endl;
    }
    if (now < t) {
        cout << now.microsecond() << " < " << t.microsecond() << endl;
    }
    if (now <= t) {
        cout << now.microsecond() << " < " << t.microsecond() << endl;
    }

    Timeval sub = Timeval() - now;
    cout << "sub.microsecond() = " << sub.microsecond() << endl;

    Timeval add = now + 1000000;
    cout << "add.microsecond() = " << add.microsecond() << endl;

    add += 1000000;
    cout << "add.microsecond() = " << add.microsecond() << endl;
    add -= 1000000;
    cout << "add.microsecond() = " << add.microsecond() << endl;

    return 0;
}