#include "RtmpPacket.hpp"

RtmpPacket::RtmpPacket()
{

}

RtmpPacket::~RtmpPacket()
{
    clear();
}

int RtmpPacket::decode_packet(char *data, int len)
{
    SrsBuffer stream;

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    while (!stream.empty()) {
        AMF0Any *any = NULL;
        if (!AmfReadAny(stream, &any)) {
            return -2;
        }

        values.push_back(any);
    }

    return 0;
}

int RtmpPacket::encode_packet(SrsBuffer &data)
{
    for (int i = 0; i < (int)values.size(); ++i) {
        AMF0Any *any = values.at(i);
        if (!AmfWriteAny(data, any)) {
            return -1;
        }
    }

    return 0;
}

void RtmpPacket::clear()
{
    for (int i = 0; i < (int)values.size(); ++i) {
        srs_freep(values.at(i));
    }
    values.clear();
}

bool RtmpPacket::findString(int num, string &value)
{
    int count = 1;
    num = (num <= 0) ? 1 : num;

    for (int i = 0; i < (int)values.size(); ++i) {
        AMF0Any *any = values.at(i);
        if (any->isShortString()) {
            if (num == count) {
                AMF0String *str = dynamic_cast<AMF0String*>(any);
                value = str->value;
                return true;
            }
            count++;
        }
    }

    return false;
}

bool RtmpPacket::findDouble(int num, double &value)
{
    int count = 1;
    num = (num <= 0) ? 1 : num;

    for (int i = 0; i < (int)values.size(); ++i) {
        AMF0Any *any = values.at(i);
        if (any->isNumber()) {
            if (num == count) {
                AMF0Number *str = dynamic_cast<AMF0Number*>(any);
                value = str->value;
                return true;
            }
            count++;
        }
    }

    return false;
}

bool RtmpPacket::findString(const string &name, string &value)
{
    for (int i = 0; i < (int)values.size(); ++i) {
        if (findStringFromAny(values.at(i), name, value)) {
            return true;
        }
    }

    return false;
}

bool RtmpPacket::findDouble(const string &name, double &value)
{
    for (int i = 0; i < (int)values.size(); ++i) {
        if (findDoubleFromAny(values.at(i), name, value)) {
            return true;
        }
    }

    return false;
}

bool RtmpPacket::replaceString(const string &src, const string &dst)
{
    for (int i = 0; i < (int)values.size(); ++i) {
        if (replaceStringToAny(values.at(i), src, dst)) {
            return true;
        }
    }

    return false;
}

bool RtmpPacket::replaceDouble(const string &src, double dst)
{
    for (int i = 0; i < (int)values.size(); ++i) {
        if (replaceDoubleToAny(values.at(i), src, dst)) {
            return true;
        }
    }

    return false;
}

bool RtmpPacket::replaceString(int num, const string &dst)
{
    int count = 1;
    num = (num <= 0) ? 1 : num;

    for (int i = 0; i < (int)values.size(); ++i) {
        AMF0Any *any = values.at(i);
        if (any->isShortString()) {
            if (num == count) {
                AMF0String *str = dynamic_cast<AMF0String*>(any);
                str->value = dst;
                return true;
            }
            count++;
        }
    }

    return false;
}

bool RtmpPacket::replaceDouble(int num, double dst)
{
    int count = 1;
    num = (num <= 0) ? 1 : num;

    for (int i = 0; i < (int)values.size(); ++i) {
        AMF0Any *any = values.at(i);
        if (any->isNumber()) {
            if (num == count) {
                AMF0Number *str = dynamic_cast<AMF0Number*>(any);
                str->value = dst;
                return true;
            }
            count++;
        }
    }

    return false;
}

bool RtmpPacket::findStringFromAny(AMF0Any *any, const string &name, string &value)
{
    if (any->isAmf0Object()) {
        AMF0Object *obj = dynamic_cast<AMF0Object*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            Amf0ObjectProperty &p = obj->values.at(i);
            string k = p.first;
            AMF0Any *v = p.second;

            if (k == name && v->isShortString()) {
                AMF0String *str = dynamic_cast<AMF0String*>(v);
                value = str->value;
                return true;
            } else if (k == name && !v->isShortString()) {
                return false;
            } else if (k != name) {
                if (findStringFromAny(v, name, value)) {
                    return true;
                }
            }
        }
    } else if (any->isEcmaArray()) {
        AMF0EcmaArray *obj = dynamic_cast<AMF0EcmaArray*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            Amf0ObjectProperty &p = obj->values.at(i);
            string k = p.first;
            AMF0Any *v = p.second;

            if (k == name && v->isShortString()) {
                AMF0String *str = dynamic_cast<AMF0String*>(v);
                value = str->value;
                return true;
            } else if (k == name && !v->isShortString()) {
                return false;
            } else if (k != name) {
                if (findStringFromAny(v, name, value)) {
                    return true;
                }
            }
        }
    } else if (any->isStrictArray()) {
        AMF0StrictArray *obj = dynamic_cast<AMF0StrictArray*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            if (findStringFromAny(obj->values.at(i), name, value)) {
                return true;
            }
        }
    }

    return false;
}

bool RtmpPacket::findDoubleFromAny(AMF0Any *any, const string &name, double &value)
{
    if (any->isAmf0Object()) {
        AMF0Object *obj = dynamic_cast<AMF0Object*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            Amf0ObjectProperty &p = obj->values.at(i);
            string k = p.first;
            AMF0Any *v = p.second;

            if (k == name && v->isNumber()) {
                AMF0Number *str = dynamic_cast<AMF0Number*>(v);
                value = str->value;
                return true;
            } else if (k == name && !v->isNumber()) {
                return false;
            } else if (k != name) {
                if (findDoubleFromAny(v, name, value)) {
                    return true;
                }
            }
        }
    } else if (any->isEcmaArray()) {
        AMF0EcmaArray *obj = dynamic_cast<AMF0EcmaArray*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            Amf0ObjectProperty &p = obj->values.at(i);
            string k = p.first;
            AMF0Any *v = p.second;

            if (k == name && v->isNumber()) {
                AMF0Number *str = dynamic_cast<AMF0Number*>(v);
                value = str->value;
                return true;
            } else if (k == name && !v->isNumber()) {
                return false;
            } else if (k != name) {
                if (findDoubleFromAny(v, name, value)) {
                    return true;
                }
            }
        }
    } else if (any->isStrictArray()) {
        AMF0StrictArray *obj = dynamic_cast<AMF0StrictArray*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            if (findDoubleFromAny(obj->values.at(i), name, value)) {
                return true;
            }
        }
    }

    return false;
}

bool RtmpPacket::replaceStringToAny(AMF0Any *any, const string &src, const string &dst)
{
    if (any->isAmf0Object()) {
        AMF0Object *obj = dynamic_cast<AMF0Object*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            Amf0ObjectProperty &p = obj->values.at(i);
            string k = p.first;
            AMF0Any *v = p.second;

            if (k == src && v->isShortString()) {
                AMF0String *str = dynamic_cast<AMF0String*>(v);
                str->value = dst;
                return true;
            } else if (k == src && !v->isShortString()) {
                return false;
            } else if (k != src) {
                if (replaceStringToAny(v, src, dst)) {
                    return true;
                }
            }
        }
    } else if (any->isEcmaArray()) {
        AMF0EcmaArray *obj = dynamic_cast<AMF0EcmaArray*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            Amf0ObjectProperty &p = obj->values.at(i);
            string k = p.first;
            AMF0Any *v = p.second;

            if (k == src && v->isShortString()) {
                AMF0String *str = dynamic_cast<AMF0String*>(v);
                str->value = dst;
                return true;
            } else if (k == src && !v->isShortString()) {
                return false;
            } else if (k != src) {
                if (replaceStringToAny(v, src, dst)) {
                    return true;
                }
            }
        }
    } else if (any->isStrictArray()) {
        AMF0StrictArray *obj = dynamic_cast<AMF0StrictArray*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            if (replaceStringToAny(obj->values.at(i), src, dst)) {
                return true;
            }
        }
    }

    return false;
}

bool RtmpPacket::replaceDoubleToAny(AMF0Any *any, const string &src, double dst)
{
    if (any->isAmf0Object()) {
        AMF0Object *obj = dynamic_cast<AMF0Object*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            Amf0ObjectProperty &p = obj->values.at(i);
            string k = p.first;
            AMF0Any *v = p.second;

            if (k == src && v->isNumber()) {
                AMF0Number *str = dynamic_cast<AMF0Number*>(v);
                str->value = dst;
                return true;
            } else if (k == src && !v->isNumber()) {
                return false;
            } else if (k != src) {
                if (replaceDoubleToAny(v, src, dst)) {
                    return true;
                }
            }
        }
    } else if (any->isEcmaArray()) {
        AMF0EcmaArray *obj = dynamic_cast<AMF0EcmaArray*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            Amf0ObjectProperty &p = obj->values.at(i);
            string k = p.first;
            AMF0Any *v = p.second;

            if (k == src && v->isNumber()) {
                AMF0Number *str = dynamic_cast<AMF0Number*>(v);
                str->value = dst;
                return true;
            } else if (k == src && !v->isNumber()) {
                return false;
            } else if (k != src) {
                if (replaceDoubleToAny(v, src, dst)) {
                    return true;
                }
            }
        }
    } else if (any->isStrictArray()) {
        AMF0StrictArray *obj = dynamic_cast<AMF0StrictArray*>(any);
        for (unsigned int i = 0; i < obj->values.size(); ++i) {
            if (replaceDoubleToAny(obj->values.at(i), src, dst)) {
                return true;
            }
        }
    }

    return false;
}

/**********************************************************************/

ConnectAppPacket::ConnectAppPacket()
{
    command_name = RTMP_AMF0_COMMAND_CONNECT;
    transaction_id = 1;
    objectEncoding = 0;

    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

ConnectAppPacket::~ConnectAppPacket()
{

}

int ConnectAppPacket::decode()
{
    /********************************/
    char *data = (char*)payload->GetBuf()->data;
    int len = size;

    if (is_amf3_command() && size >= 1) {
        data = (char*)payload->GetBuf()->data + 1;
        len = size - 1;
    }

    if (decode_packet(data, len) != 0) {
        return -1;
    }
    /********************************/

    if (!findString(1, command_name)) {
        return -2;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_CONNECT) {
        return -3;
    }

    if (findDouble(1, transaction_id)) {
        if (transaction_id != 1.0) {
            return -4;
        }
    } else {
        return -5;
    }

    if (!findString("tcUrl", tcUrl)) {
        return -6;
    }

    findDouble("objectEncoding", objectEncoding);
    findString("pageUrl", pageUrl);
    findString("swfUrl", swfUrl);

    return 0;
}

int ConnectAppPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!values.empty()) {
        if (encode_packet(stream) != 0) {
            return -1;
        }
    } else {
        if (!AmfWriteString(stream, command_name)) {
            return -2;
        }
        // transaction_id
        if (!AmfWriteDouble(stream, 1)) {
            return -3;
        }
        if (!AmfWriteObject(stream, command_object)) {
            return -4;
        }
    }

    return 0;
}

int ConnectAppPacket::get_size()
{
    int size = 0;

    size += SrsAmf0Size::str(command_name);
    size += SrsAmf0Size::number();
    size += SrsAmf0Size::object(&command_object);

    return size;
}

/**********************************************************************/

ConnectAppResPacket::ConnectAppResPacket()
{
    command_name = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = 1;

    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

ConnectAppResPacket::~ConnectAppResPacket()
{

}

int ConnectAppResPacket::decode()
{
    if (decode_packet((char*)payload->GetBuf()->data, size) != 0) {
        return -1;
    }

    if (!findString(1, command_name)) {
        return -2;
    }

    if (command_name.empty() || (command_name != RTMP_AMF0_COMMAND_RESULT
                                 && command_name != RTMP_AMF0_COMMAND_ERROR)) {
        return -3;
    }

    if (findDouble(1, transaction_id)) {
        if (transaction_id != 1.0) {
            return -4;
        }
    } else {
        return -5;
    }

    return 0;
}

int ConnectAppResPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }

    // transaction_id
    if (!AmfWriteDouble(stream, 1)) {
        return -3;
    }

    if (!AmfWriteObject(stream, props)) {
        return -4;
    }

    if (!AmfWriteObject(stream, info)) {
        return -5;
    }

    return 0;
}

int ConnectAppResPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
    + SrsAmf0Size::object(&props) + SrsAmf0Size::object(&info);
}

/**********************************************************************/
CreateStreamPacket::CreateStreamPacket()
{
    command_name = RTMP_AMF0_COMMAND_CREATE_STREAM;
    transaction_id = 2;

    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

CreateStreamPacket::~CreateStreamPacket()
{

}

int CreateStreamPacket::decode()
{
    /********************************/
    char *data = (char*)payload->GetBuf()->data;
    int len = size;

    if (is_amf3_command() && size >= 1) {
        data = (char*)payload->GetBuf()->data + 1;
        len = size - 1;
    }

    if (decode_packet(data, len) != 0) {
        return -1;
    }
    /********************************/

    if (!findString(1, command_name)) {
        return -2;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_CREATE_STREAM) {
        return -3;
    }

    if (!findDouble(1, transaction_id)) {
        return -4;
    }

    return 0;
}

int CreateStreamPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteDouble(stream, transaction_id)) {
        return -3;
    }
    if (!AmfWriteNull(stream)) {
        return -4;
    }

    return 0;
}

int CreateStreamPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
    + SrsAmf0Size::null();
}

/**********************************************************************/
CreateStreamResPacket::CreateStreamResPacket(double _transaction_id, double _stream_id)
{
    command_name = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = _transaction_id;
    stream_id = _stream_id;

    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

CreateStreamResPacket::~CreateStreamResPacket()
{

}

int CreateStreamResPacket::decode()
{
    if (decode_packet((char*)payload->GetBuf()->data, size) != 0) {
        return -1;
    }

    if (!findString(1, command_name)) {
        return -2;
    }

    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT) {
        return -3;
    }

    if (!findDouble(1, transaction_id)) {
        return -4;
    }

    if (!findDouble(2, stream_id)) {
        return -5;
    }

    return 0;
}

int CreateStreamResPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteDouble(stream, transaction_id)) {
        return -3;
    }
    if (!AmfWriteNull(stream)) {
        return -4;
    }
    if (!AmfWriteDouble(stream, stream_id)) {
        return -5;
    }

    return 0;
}

int CreateStreamResPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
    + SrsAmf0Size::null() + SrsAmf0Size::number();
}

/**********************************************************************/
SetChunkSizePacket::SetChunkSizePacket()
{
    chunk_size = 128;
    header.perfer_cid = RTMP_CID_ProtocolControl;
    header.message_type = RTMP_MSG_SetChunkSize;
}

SetChunkSizePacket::~SetChunkSizePacket()
{

}

int SetChunkSizePacket::decode()
{
    SrsBuffer stream;

    if (stream.initialize((char*)payload->GetBuf()->data, size) != 0) {
        return -1;
    }

    if (!stream.require(4)) {
        return -2;
    }

    chunk_size = stream.read_4bytes();

    return 0;
}

int SetChunkSizePacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    stream.write_4bytes(chunk_size);

    return 0;
}

int SetChunkSizePacket::get_size()
{
    return 4;
}

/**********************************************************************/
AcknowledgementPacket::AcknowledgementPacket()
{
    sequence_number = 0;
    header.perfer_cid = RTMP_CID_ProtocolControl;
    header.message_type = RTMP_MSG_Acknowledgement;
}

AcknowledgementPacket::~AcknowledgementPacket()
{

}

int AcknowledgementPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    stream.write_4bytes(sequence_number);

    return 0;
}

int AcknowledgementPacket::get_size()
{
    return 4;
}

/**********************************************************************/
SetWindowAckSizePacket::SetWindowAckSizePacket()
{
    ackowledgement_window_size = 0;
    header.perfer_cid = RTMP_CID_ProtocolControl;
    header.message_type = RTMP_MSG_WindowAcknowledgementSize;
}

SetWindowAckSizePacket::~SetWindowAckSizePacket()
{

}

int SetWindowAckSizePacket::decode()
{
    SrsBuffer stream;

    if (stream.initialize((char*)payload->GetBuf()->data, size) != 0) {
        return -1;
    }

    if (!stream.require(4)) {
        return -2;
    }

    ackowledgement_window_size = stream.read_4bytes();

    return 0;
}

int SetWindowAckSizePacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    stream.write_4bytes(ackowledgement_window_size);

    return 0;
}

int SetWindowAckSizePacket::get_size()
{
    return 4;
}

/**********************************************************************/
OnErrorPacket::OnErrorPacket()
{
    command_name = RTMP_AMF0_COMMAND_ERROR;
    transaction_id = 1;
    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

OnErrorPacket::~OnErrorPacket()
{

}

int OnErrorPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteDouble(stream, transaction_id)) {
        return -3;
    }
    if (!AmfWriteNull(stream)) {
        return -4;
    }
    if (!AmfWriteObject(stream, error_info)) {
        return -5;
    }

    return 0;
}

int OnErrorPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
    + SrsAmf0Size::null() + SrsAmf0Size::object(&error_info);
}

/**********************************************************************/

CallPacket::CallPacket()
{
    transaction_id = 0;
    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

CallPacket::~CallPacket()
{

}

int CallPacket::decode()
{
    if (decode_packet((char*)payload->GetBuf()->data, size) != 0) {
        return -1;
    }

    if (!findString(1, command_name)) {
        return -2;
    }
    if (!findDouble(1, transaction_id)) {
        return -3;
    }

    return 0;
}

/**********************************************************************/

CallResPacket::CallResPacket(double _transaction_id)
{
    transaction_id = _transaction_id;
    command_name = RTMP_AMF0_COMMAND_RESULT;

    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;

    command_object = response = NULL;
}

CallResPacket::~CallResPacket()
{

}

int CallResPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteDouble(stream, transaction_id)) {
        return -3;
    }
    if (command_object && AmfWriteAny(stream, command_object)) {
        return -4;
    }
    if (response && AmfWriteAny(stream, response)) {
        return -5;
    }

    return 0;
}

int CallResPacket::get_size()
{
    int size = 0;

    size += SrsAmf0Size::str(command_name) + SrsAmf0Size::number();

    if (command_object) {
        size += command_object->total_size();
    }

    if (response) {
        size += response->total_size();
    }

    return size;
}

/**********************************************************************/

CloseStreamPacket::CloseStreamPacket()
{
    command_name = RTMP_AMF0_COMMAND_CLOSE_STREAM;
    transaction_id = 0;

    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

CloseStreamPacket::~CloseStreamPacket()
{

}

int CloseStreamPacket::decode()
{
    /********************************/
    char *data = (char*)payload->GetBuf()->data;
    int len = size;

    if (is_amf3_command() && size >= 1) {
        data = (char*)payload->GetBuf()->data + 1;
        len = size - 1;
    }

    if (decode_packet(data, len) != 0) {
        return -1;
    }
    /********************************/

    if (!findString(1, command_name)) {
        return -2;
    }
    if (!findDouble(1, transaction_id)) {
        return -3;
    }

    return 0;
}

int CloseStreamPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteDouble(stream, transaction_id)) {
        return -3;
    }
    if (!AmfWriteNull(stream)) {
        return -4;
    }

    return 0;
}

int CloseStreamPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
            + SrsAmf0Size::null();
}

/**********************************************************************/

FmleStartPacket::FmleStartPacket()
{
    command_name = RTMP_AMF0_COMMAND_RELEASE_STREAM;
    transaction_id = 0;
    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

FmleStartPacket::~FmleStartPacket()
{

}

int FmleStartPacket::decode()
{
    /********************************/
    char *data = (char*)payload->GetBuf()->data;
    int len = size;

    if (is_amf3_command() && size >= 1) {
        data = (char*)payload->GetBuf()->data + 1;
        len = size - 1;
    }

    if (decode_packet(data, len) != 0) {
        return -1;
    }
    /********************************/

    if (!findString(1, command_name)) {
        return -2;
    }
    if (command_name.empty() ||
       (command_name != RTMP_AMF0_COMMAND_RELEASE_STREAM &&
        command_name != RTMP_AMF0_COMMAND_FC_PUBLISH &&
        command_name != RTMP_AMF0_COMMAND_UNPUBLISH))
    {
        return -3;
    }

    if (!findDouble(1, transaction_id)) {
        return -4;
    }

    return 0;
}

/**********************************************************************/

FmleStartResPacket::FmleStartResPacket(double _transaction_id)
{
    command_name = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = _transaction_id;
    header.perfer_cid = RTMP_CID_OverConnection;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

FmleStartResPacket::~FmleStartResPacket()
{

}

int FmleStartResPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteDouble(stream, transaction_id)) {
        return -3;
    }
    if (!AmfWriteNull(stream)) {
        return -4;
    }
    if (!AmfWriteUndefined(stream)) {
        return -5;
    }

    return 0;
}

int FmleStartResPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
    + SrsAmf0Size::null() + SrsAmf0Size::undefined();
}

/**********************************************************************/

PublishPacket::PublishPacket()
{
    command_name = RTMP_AMF0_COMMAND_PUBLISH;
    transaction_id = 0;
    type = "live";
    header.perfer_cid = RTMP_CID_OverStream;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

PublishPacket::~PublishPacket()
{

}

int PublishPacket::decode()
{
    /********************************/
    char *data = (char*)payload->GetBuf()->data;
    int len = size;

    if (is_amf3_command() && size >= 1) {
        data = (char*)payload->GetBuf()->data + 1;
        len = size - 1;
    }

    if (decode_packet(data, len) != 0) {
        return -1;
    }
    /********************************/

    if (!findString(1, command_name)) {
        return -2;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PUBLISH) {
        return -3;
    }

    if (!findDouble(1, transaction_id)) {
        return -4;
    }

    if (!findString(2, stream_name)) {
        return -5;
    }

    if (stream_name.empty()) {
        return -6;
    }

    return 0;
}

int PublishPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteDouble(stream, transaction_id)) {
        return -3;
    }
    if (!AmfWriteNull(stream)) {
        return -4;
    }
    if (!AmfWriteString(stream, stream_name)) {
        return -5;
    }
    if (!AmfWriteString(stream, type)) {
        return -6;
    }

    return 0;
}

int PublishPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
    + SrsAmf0Size::null() + SrsAmf0Size::str(stream_name)
    + SrsAmf0Size::str(type);
}

/**********************************************************************/

PlayPacket::PlayPacket()
{
    command_name = RTMP_AMF0_COMMAND_PLAY;
    transaction_id = 0;
    header.perfer_cid = RTMP_CID_OverStream;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

PlayPacket::~PlayPacket()
{

}

int PlayPacket::decode()
{
    /********************************/
    char *data = (char*)payload->GetBuf()->data;
    int len = size;

    if (is_amf3_command() && size >= 1) {
        data = (char*)payload->GetBuf()->data + 1;
        len = size - 1;
    }

    if (decode_packet(data, len) != 0) {
        return -1;
    }
    /********************************/

    if (!findString(1, command_name)) {
        return -2;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PLAY) {
        return -3;
    }

    if (!findDouble(1, transaction_id)) {
        return -4;
    }

    if (!findString(2, stream_name)) {
        return -5;
    }
    if (stream_name.empty()) {
        return -6;
    }

    return 0;
}

int PlayPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteDouble(stream, transaction_id)) {
        return -3;
    }
    if (!AmfWriteNull(stream)) {
        return -4;
    }
    if (!AmfWriteString(stream, stream_name)) {
        return -5;
    }

    return 0;
}

int PlayPacket::get_size()
{
    int size = SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
    + SrsAmf0Size::null() + SrsAmf0Size::str(stream_name);

    return size;
}

/**********************************************************************/

OnStatusCallPacket::OnStatusCallPacket()
{
    command_name = RTMP_AMF0_COMMAND_ON_STATUS;
    transaction_id = 0;
    header.perfer_cid = RTMP_CID_OverStream;
    header.message_type = RTMP_MSG_AMF0CommandMessage;
}

OnStatusCallPacket::~OnStatusCallPacket()
{

}

int OnStatusCallPacket::decode()
{
    if (decode_packet((char*)payload->GetBuf()->data, size) != 0) {
        return -1;
    }

    if (!findString(1, command_name)) {
        return -2;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_ON_STATUS) {
        return -3;
    }

    if (!findDouble(1, transaction_id)) {
        return -4;
    }

    if(!findString(StatusCode, status_code)) {
        return -5;
    }

    return 0;
}

int OnStatusCallPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteDouble(stream, transaction_id)) {
        return -3;
    }
    if (!AmfWriteNull(stream)) {
        return -4;
    }
    if (!AmfWriteObject(stream, value)) {
        return -5;
    }

    return 0;
}

int OnStatusCallPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
    + SrsAmf0Size::null() + SrsAmf0Size::object(&value);
}

/**********************************************************************/

OnStatusDataPacket::OnStatusDataPacket()
{
    command_name = RTMP_AMF0_COMMAND_ON_STATUS;
    header.perfer_cid = RTMP_CID_OverStream;
    header.message_type = RTMP_MSG_AMF0DataMessage;
}

OnStatusDataPacket::~OnStatusDataPacket()
{

}

int OnStatusDataPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteObject(stream, value)) {
        return -3;
    }

    return 0;
}

int OnStatusDataPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::object(&value);
}

/**********************************************************************/

SampleAccessPacket::SampleAccessPacket()
{
    command_name = RTMP_AMF0_DATA_SAMPLE_ACCESS;
    video_sample_access = false;
    audio_sample_access = false;
    header.perfer_cid = RTMP_CID_OverStream;
    header.message_type = RTMP_MSG_AMF0DataMessage;
}

SampleAccessPacket::~SampleAccessPacket()
{

}

int SampleAccessPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, command_name)) {
        return -2;
    }
    if (!AmfWriteBoolean(stream, video_sample_access)) {
        return -3;
    }
    if (!AmfWriteBoolean(stream, audio_sample_access)) {
        return -4;
    }

    return 0;
}

int SampleAccessPacket::get_size()
{
    return SrsAmf0Size::str(command_name)
    + SrsAmf0Size::boolean() + SrsAmf0Size::boolean();
}

/**********************************************************************/

UserControlPacket::UserControlPacket()
{
    event_type = 0;
    event_data = 0;
    extra_data = 0;
    header.perfer_cid = RTMP_CID_ProtocolControl;
    header.message_type = RTMP_MSG_UserControlMessage;
}

UserControlPacket::~UserControlPacket()
{

}

int UserControlPacket::decode()
{
    SrsBuffer stream;

    if (stream.initialize((char*)payload->GetBuf()->data, size) != 0) {
        return -1;
    }

    if (!stream.require(2)) {
        return -2;
    }

    event_type = stream.read_2bytes();

    if (event_type != SrcPCUCSWFVerifyResponse || event_type != SrcPCUCSWFVerifyRequest) {
        if (!stream.require(4)) {
            return -3;
        }
        event_data = stream.read_4bytes();

        if (event_type == SrcPCUCSetBufferLength) {
            if (!stream.require(4)) {
                return -3;
            }
            extra_data = stream.read_4bytes();
        }
    }

    return 0;
}

int UserControlPacket::encode()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    stream.write_2bytes(event_type);

    if (event_type != SrcPCUCSWFVerifyResponse || event_type != SrcPCUCSWFVerifyRequest) {
        stream.write_4bytes(event_data);

        if (event_type == SrcPCUCSetBufferLength) {
            stream.write_4bytes(extra_data);
        }
    }

    return 0;
}

int UserControlPacket::get_size()
{
    int size = 2;

    if (event_type == SrcPCUCSWFVerifyRequest) {
        size += 1;
    } else {
        size += 4;
    }

    if (event_type == SrcPCUCSetBufferLength) {
        size += 4;
    }

    return size;
}

/**********************************************************************/

OnMetaDataPacket::OnMetaDataPacket()
{
    num = 0;
    name = RTMP_AMF0_DATA_ON_METADATA;
    header.perfer_cid = RTMP_CID_OverConnection2;
    header.message_type = RTMP_MSG_AMF0DataMessage;
}

OnMetaDataPacket::~OnMetaDataPacket()
{

}

int OnMetaDataPacket::decode()
{
    /********************************/
    char *data = (char*)payload->GetBuf()->data;
    int len = size;

    if (is_amf3_data() && size >= 1) {
        data = (char*)payload->GetBuf()->data + 1;
        len = size - 1;
    }

    if (decode_packet(data, len) != 0) {
        char *body = new char[len];
        unique_ptr<char> p(body);
        memcpy(body, data, len);

        if ((data = generate_payload(len)) == NULL) { return -1; }
        memcpy(data, body, len);

        return 0;
    }
    /********************************/

    if (!findString(1, name)) {
        return -2;
    }

    num++;

    // ignore the @setDataFrame
    if (name == RTMP_AMF0_DATA_SET_DATAFRAME) {
        if (!findString(2, name)) {
            return -3;
        }
        num++;
    }

    if (encode_body() != 0) {
        return -4;
    }

    return 0;
}

int OnMetaDataPacket::get_size()
{
    int size = SrsAmf0Size::str(name);

    for (int i = num; i < (int)values.size(); ++i) {
        AMF0Any *any = values.at(i);

        if (any->isAmf0Object()) {
            size += 4;
        }
        size += any->total_size();
    }

    return size;
}

int OnMetaDataPacket::encode_body()
{
    SrsBuffer stream;

    int len = get_size();
    char *data = NULL;
    if ((data = generate_payload(len)) == NULL) { return -1; }

    if (stream.initialize(data, len) != 0) {
        return -1;
    }

    if (!AmfWriteString(stream, name)) {
        return -2;
    }

    for (int i = num; i < (int)values.size(); ++i) {
        AMF0Any *metadata = values.at(i);

        if (metadata->isEcmaArray()) {
            AMF0EcmaArray *data = dynamic_cast<AMF0EcmaArray*>(metadata);
            if (!AmfWriteEcmaArray(stream, *data)) {
                return -3;
            }
        } else if (metadata->isAmf0Object()) {
            AMF0Object *data = dynamic_cast<AMF0Object*>(metadata);
            if (!AmfWriteEcmaArray(stream, *data)) {
                return -4;
            }
        } else {
            if (!AmfWriteAny(stream, metadata)) {
                return -5;
            }
        }
    }

    return 0;
}
