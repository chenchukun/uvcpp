#ifndef LIBUVCPP_CODEC_H
#define LIBUVCPP_CODEC_H

#include "utility.h"
#include <string>
#include <netinet/in.h>
#include <google/protobuf/message.h>
#include "Buffer.h"
#include "BufferStream.h"

NAMESPACE_START

class Codec
{
public:
    enum {CODEC_OK = 0, CODEC_UNABLE = 1, CODEC_ERR=2};

    static int encode(const google::protobuf::Message &message, Buffer &buffer) {
        std::string result;
        std::string typeName = message.GetTypeName();
        result += typeName;
        if (!message.AppendToString(&result)) {
            return CODEC_ERR;
        }
        buffer.appendInt32(result.size() + sizeof(int32_t));
        buffer.appendInt32(typeName.size());
        buffer.append(result);
        return CODEC_OK;
    }

    static int decode(Buffer &buffer, google::protobuf::Message *&message) {
        int32_t packLen = buffer.peekInt32();
        if (packLen + sizeof(int32_t) > buffer.readableBytes()) {
            return CODEC_UNABLE;
        }
        buffer.discard(sizeof(int32_t));
        int32_t nameLen = buffer.readInt32();
        std::string typeName = buffer.retrieveAsString(nameLen);
        message = createMessage(typeName);
        if (message == NULL) {
            return CODEC_ERR;
        }
        size_t len = packLen - typeName.size() - sizeof(int32_t);
        BufferInputStream inputStream(&buffer, len);
        if (!message->ParseFromZeroCopyStream(&inputStream)) {
            return CODEC_ERR;
        }
        return CODEC_OK;
    }

    static google::protobuf::Message* createMessage(const std::string &typeName) {
        google::protobuf::Message *message = NULL;
        const google::protobuf::Descriptor *descriptor =
                google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
        if (descriptor != NULL) {
            const google::protobuf::Message *prototype =
                    google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
            if (prototype != NULL) {
                message = prototype->New();
            }
        }
        return message;
    }
};

NAMESPACE_END

#endif //LIBUVCPP_CODEC_H
