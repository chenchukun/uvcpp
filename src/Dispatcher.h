#ifndef LIBUVCPP_DISPATCHER_H
#define LIBUVCPP_DISPATCHER_H

#include "utility.h"
#include <google/protobuf/message.h>
#include <functional>
#include <map>

NAMESPACE_START

class Dispatcher
{
public:
    typedef const google::protobuf::Message* MessagePointer;
    typedef const google::protobuf::Descriptor* DescriptorPointer;
    typedef std::function<void(MessagePointer)> MessageCallback;

    Dispatcher() : defaultCallback(NULL) {

    }

    void onMessage(MessagePointer message) {
        DescriptorPointer descriptor = message->GetDescriptor();
        auto it = callbackMap.find(descriptor);
        if (it != callbackMap.end()) {
            it->second(message);
        }
        else {
            if (defaultCallback) {
                defaultCallback(message);
            }
        }
    }

    void setDefaultCallback(MessageCallback cb) {
        defaultCallback = cb;
    }

    void registerMessageCallback(const std::string &typeName, MessageCallback cb) {
        DescriptorPointer descriptor =
                google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
        callbackMap[descriptor] = cb;
    }

private:
    std::map<DescriptorPointer, MessageCallback> callbackMap;

    MessageCallback defaultCallback;
};

NAMESPACE_END

#endif //LIBUVCPP_DISPATCHER_H
