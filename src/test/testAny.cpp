//
// Created by chenchukun on 18/4/3.
//
#include "../Any.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
using namespace uvcpp;

int main()
{
    string str("hello world");
    uvcpp::Any any(str);
    string strAny;
    any.castTo(strAny);
    cout << "str = " << str << endl;
    cout << "strAny = " << strAny << endl;

    vector<string> v;
    v.push_back("hello");
    v.push_back("world");
    cout << "v = " << endl;
    for_each(v.begin(), v.end(), [] (const string &s) {cout << "\t" << s << endl;});
    any = v;
    vector<string> vAny;
    any.castTo(vAny);
    cout << "vAny = " << endl;
    for_each(vAny.begin(), vAny.end(), [] (const string &s) {cout << "\t" << s << endl;});
    return 0;
}

