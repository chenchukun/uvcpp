#include "../Buffer.h"
#include "person.pb.h"
#include <iostream>
using namespace std;
using namespace uvcpp;

int main()
{
    Buffer buffer(10);
    buffer.appendInt8(16);
    buffer.appendInt16(1024);
    cout << (int(buffer.readInt8()) == 16) << endl;
    buffer.prepend("hello uvcpp");
    cout << (buffer.retrieveAsString(11) == "hello uvcpp") << endl;
    buffer.appendInt32(66666);
    buffer.appendInt64(1234567890123);
    buffer.append("hello world");
    cout << (buffer.readInt16() == 1024) << endl;
    cout << (buffer.peekInt32() == 66666) << endl;
    buffer.discard(4);
    cout << (buffer.readInt64() == 1234567890123) << endl;
    cout << (buffer.retrieveAsString(11) == "hello world") << endl;
    buffer.prependInt8(8);
    buffer.prependInt16(16);
    buffer.prependInt32(32);
    buffer.prependInt64(64);

    cout << (buffer.readInt64() == 64) << endl;
    cout << (buffer.readInt32() == 32) << endl;
    cout << (buffer.readInt16() == 16) << endl;
    cout << (buffer.readInt8() == 8) << endl;

    Person person;
    person.set_name("kikuchanj");
    person.set_id(1);
    cout << buffer.readableBytes() << endl;
    person.SerializeToZeroCopyStream(&buffer);
    cout << buffer.readableBytes() << endl;

    return 0;
}