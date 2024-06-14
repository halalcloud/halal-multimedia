#ifndef RTMPAMF0_HPP
#define RTMPAMF0_HPP

#include "stdafx.h"
#include "SrsBuffer.hpp"
#include "RtmpGlobal.hpp"

// AMF0 types
#define AMF0_NUMBER         (0x00)
#define AMF0_BOOLEAN        (0x01)
#define AMF0_SHORT_STRING   (0x02)
#define AMF0_OBJECT         (0x03)
#define AMF0_NULL           (0x05)
#define AMF0_UNDEFINED      (0x06)
#define AMF0_ECMA_ARRAY     (0x08)
#define AMF0_OBJECT_END     (0x09)
#define AMF0_STRICT_ARRAY   (0x0a)
#define AMF0_DATE           (0x0b)
#define AMF0_LONG_STRING    (0x0c)
#define AMF0_TYPED_OBJECT	(0x10)
#define AMF0_AMF3_OBJECT    (0x11)

class SrsAmf0Size;

struct AMF0Any
{
    AMF0Any() {}
    virtual ~AMF0Any() {}

    inline bool isNumber()          { return type == AMF0_NUMBER; }
    inline bool isBoolean()         { return type == AMF0_BOOLEAN; }
    inline bool isShortString()     { return type == AMF0_SHORT_STRING; }
    inline bool isAmf0Object()      { return type == AMF0_OBJECT; }
    inline bool isNull()            { return type == AMF0_NULL; }
    inline bool isUndefined()       { return type == AMF0_UNDEFINED; }
    inline bool isEcmaArray()       { return type == AMF0_ECMA_ARRAY; }
    inline bool isStrictArray()     { return type == AMF0_STRICT_ARRAY; }

    virtual int total_size() = 0;

    char type;
};

struct AMF0Number : public AMF0Any
{
    AMF0Number(double v = 0.0)
    {
        type = AMF0_NUMBER;
        value = v;
    }
    virtual ~AMF0Number() {}

    virtual int total_size();

    double value;
};

struct AMF0Boolean : public AMF0Any
{
    AMF0Boolean(bool v = false)
    {
        type = AMF0_BOOLEAN;
        value = v;
    }
    virtual ~AMF0Boolean() {}

    virtual int total_size();

    bool value;
};

struct AMF0String : public AMF0Any
{
    AMF0String(const string &v = "")
    {
        type = AMF0_SHORT_STRING;
        value = v;
    }
    virtual ~AMF0String() {}

    virtual int total_size();

    string value;
};

typedef pair<string, AMF0Any*> Amf0ObjectProperty;
struct AMFObject : public AMF0Any
{
    AMFObject() {}
    virtual ~AMFObject() { clear(); }

    string key(int index);
    AMF0Any *value(int index);
    string value(const string &key);
    AMF0Any *query(const string &key);
    int indexOf(const string &key);
    void setValue(const string &key, AMF0Any *any);
    void clear();
    bool empty();

    vector<Amf0ObjectProperty> values;
};

struct AMF0Object : public AMFObject
{
    AMF0Object()
    {
        type = AMF0_OBJECT;
    }
    virtual ~AMF0Object() { clear(); }

    virtual int total_size();
};

struct AMF0Null : public AMF0Any
{
    AMF0Null()
    {
        type = AMF0_NULL;
    }
    virtual ~AMF0Null() {}

    virtual int total_size();
};

struct AMF0Undefined : public AMF0Any
{
    AMF0Undefined()
    {
        type = AMF0_UNDEFINED;
    }
    virtual ~AMF0Undefined() {}

    virtual int total_size();
};

struct AMF0EcmaArray : public AMFObject
{
    AMF0EcmaArray()
    {
        type = AMF0_ECMA_ARRAY;
    }
    virtual ~AMF0EcmaArray() { clear(); }

    virtual int total_size();

    uint32_t count;
};

struct AMF0StrictArray : public AMF0Any
{
    AMF0StrictArray()
    {
        type = AMF0_STRICT_ARRAY;
    }
    virtual ~AMF0StrictArray() { clear(); }

    void clear();

    virtual int total_size();

    vector<AMF0Any*> values;
    uint32_t count;
};

struct AMF0Date : public AMF0Any
{
    AMF0Date()
    {
        type = AMF0_DATE;
    }
    virtual ~AMF0Date() {}

    virtual int total_size();

    int64_t date_value;
    int16_t time_zone;
};

/*********************************************************/

bool AmfReadString(SrsBuffer &buffer, string &var);
bool AmfReadDouble(SrsBuffer &buffer, double &var);
bool AmfReadBoolean(SrsBuffer &buffer, bool &var);
bool AmfReadNull(SrsBuffer &buffer);
bool AmfReadUndefined(SrsBuffer &buffer);
bool AmfReadObject(SrsBuffer &buffer, AMF0Object &var);
bool AmfReadEcmaArray(SrsBuffer &buffer, AMF0EcmaArray &var);
bool AmfReadStrictArray(SrsBuffer &buffer, AMF0StrictArray &var);
bool AmfReadDate(SrsBuffer &buffer, AMF0Date &var);
bool AmfReadAny(SrsBuffer &buffer, AMF0Any **var);

/*********************************************************/

bool AmfWriteString(SrsBuffer &buffer, const string &var);
bool AmfWriteDouble(SrsBuffer &buffer, double value);
bool AmfWriteBoolean(SrsBuffer &buffer, bool value);
bool AmfWriteNull(SrsBuffer &buffer);
bool AmfWriteUndefined(SrsBuffer &buffer);
bool AmfWriteObject(SrsBuffer &buffer, AMF0Object &var);
bool AmfWriteEcmaArray(SrsBuffer &buffer, AMF0EcmaArray &var);
bool AmfWriteStrictArray(SrsBuffer &buffer, AMF0StrictArray &var);
bool AmfWriteDate(SrsBuffer &buffer, AMF0Date &var);
bool AmfWriteAny(SrsBuffer &buffer, AMF0Any *any);

/**
 * @brief 生成metadata时使用
 * @param buffer
 * @param var
 * @return true or false
 */
bool AmfWriteEcmaArray(SrsBuffer &buffer, AMF0Object &var);

/*********************************************************/

class SrsAmf0Size
{
public:
    static int utf8(string value);
    static int str(string value);
    static int number();
    static int date();
    static int null();
    static int undefined();
    static int boolean();
    static int object(AMF0Object* obj);
    static int object_eof();
    static int ecma_array(AMF0EcmaArray* arr);
    static int strict_array(AMF0StrictArray* arr);
    static int any(AMF0Any* o);
};

#endif // RTMPAMF0_HPP
