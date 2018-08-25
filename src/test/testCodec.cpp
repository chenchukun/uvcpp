#include "../Codec.h"
#include "../Dispatcher.h"
#include "../Buffer.h"
#include "person.pb.h"
#include <iostream>
using namespace std;
using namespace uvcpp;

int main()
{
    Person person;
    person.set_name("kikuchanj");
    person.set_id(618);
    cout << person.DebugString() << endl;
    Buffer buffer;
    int ret = Codec::encode(person, buffer);
    if (ret != 0) {
        cerr << "encode error: " << ret << endl;
        exit(ret);
    }
    google::protobuf::Message *message;
    ret = Codec::decode(buffer, message);
    if (ret != 0) {
        cerr << "decode error: " << ret << endl;
        exit(ret);
    }

    Dispatcher dispatcher;
    dispatcher.registerMessageCallback(message->GetTypeName(), [] (Dispatcher::MessagePointer message) {
        cout << message->DebugString() << endl;
    });

    dispatcher.onMessage(message);

    delete message;

    return 0;
}