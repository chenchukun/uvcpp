#include "../Buffer.h"
#include "../BufferStream.h"
#include "person.pb.h"
#include <iostream>
using namespace std;
using namespace uvcpp;

string result;

const char* chars = "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_,./:*+;[]-=~^|'\"<>";

size_t randInt(size_t low, size_t hight)
{
    size_t n = (rand() % (hight - low)) + low;
    return n;
}

string randString(size_t len)
{
    string str;
    str.reserve(len);
    while (len--) {
        str.push_back(chars[randInt(0, strlen(chars))]);
    }
    return str;
}

void append(Buffer &buffer, size_t len)
{
    cout << "append " << len << " bytes" << endl;
    string str = move(randString(len));
    buffer.append(str);
    result += str;
    buffer.debug();
    if (buffer.toString() != result) {
        cerr << "ERROR: append error" << endl;
        cerr << "append str = " << str << endl;
        cerr << "result = " << result << endl;
        cerr << "buffer = " << buffer.toString() << endl;
        exit(-1);
    }
}

void prepend(Buffer &buffer, size_t len)
{
    cout << "prepend " << len << " bytes" << endl;
    string str = move(randString(len));
    buffer.prepend(str);
    result = str + result;
    buffer.debug();
    if (buffer.toString() != result) {
        cerr << "ERROR: prepend error" << endl;
        cerr << "prepend str = " << str << endl;
        cerr << "result = " << result << endl;
        cerr << "buffer = " << buffer.toString() << endl;
        exit(-1);
    }
}

void retrieveAsString(Buffer &buffer, size_t len)
{
    cout << "retrieveAsString " << len << " bytes" << endl;
    string str = buffer.retrieveAsString(len);
    string res = result.substr(0, len);
    result = result.substr(len, result.size() - len);
    buffer.debug();
    if (str != res) {
        cerr << "ERROR: retrieveAsString error" << endl;
        cerr << "retrieveAsString len = " << len << endl;
        cerr << "res = " << res << endl;
        cerr << "str = " << str << endl;
        exit(-1);
    }
}

void discard(Buffer &buffer, size_t len)
{
    cout << "discard " << len << " bytes" << endl;
    size_t readableBytes = buffer.readableBytes();
    buffer.discard(len);
    result = result.substr(len, result.size() - len);
    buffer.debug();
    if (buffer.readableBytes() != readableBytes - len) {
        cerr << "ERROR: discard error" << endl;
        cerr << "discard len = " << len << endl;
        exit(-1);
    }
}

void test(int n)
{
    Buffer buffer;

    srand(time(NULL));
    for (int i=0; i<n; ++i) {
        size_t chose = randInt(0, 70);
        if (chose < 30) {
            if (buffer.readableBytes() < 409600) {
                append(buffer, randInt(1, 2048));
            }
        }
        else if (chose < 35) {
            if (buffer.readableBytes() < 409600) {
                prepend(buffer, randInt(1, 512));
            }
        }
        else if (chose < 60) {
            if (buffer.readableBytes() > 0) {
                retrieveAsString(buffer, randInt(1, buffer.readableBytes() + 1));
            }
        }
        else {
            if (buffer.readableBytes() > 0) {
                discard(buffer, randInt(1, buffer.readableBytes() + 1));
            }
        }
    }
}

void testCopy()
{
    Buffer buffer;
    buffer.append(randString(500));
    buffer.prepend("12345", 5);
    buffer.append(randString(600));
    buffer.debug();
    Buffer buffer2 = buffer;
    buffer2.debug();
    Buffer buffer3 = move(buffer2);
    buffer2.debug();
    buffer3.debug();

    Buffer buffer4;
    buffer4.swap(buffer3);
    buffer3.debug();
    buffer4.debug();
}

void testStream()
{
    Buffer buffer;
    Person person;
    person.set_name("kiku");
    person.set_id(6180);
    BufferOutputStream outputStream(&buffer);
    person.SerializeToZeroCopyStream(&outputStream);
    cout << buffer.toString() << endl;

    Person person1;
    person1.ParseFromString(buffer.toString());
    cout << person1.DebugString() << endl;

    BufferInputStream inputStream(&buffer, buffer.readableBytes());
    Person person2;
    person2.ParseFromZeroCopyStream(&inputStream);
    cout << person2.DebugString() << endl;
}

int main(int argc, char **argv)
{
//    test(stoi(argv[1]));
//    testCopy();
    testStream();

    return 0;
}