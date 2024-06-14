#include "RtmpAmf0.hpp"

AMF0Any *AMFObject::value(int index)
{
    if (index >= (int)values.size()) {
        return NULL;
    }

    pair<string, AMF0Any*> &p = values.at(index);
    return p.second;
}

string AMFObject::key(int index)
{
    if (index >= (int)values.size()) {
        return "";
    }

    pair<string, AMF0Any *> &p = values.at(index);
    return p.first;
}

string AMFObject::value(const string &key)
{
    int index = indexOf(key);
    if (index >= 0) {
        AMF0Any *any = value(index);
        if (!any) {
            return "";
        }

        AMF0String *str = dynamic_cast<AMF0String*>(any);
        if (!str) {
            return "";
        }
        return str->value;
    }

    return "";
}

AMF0Any *AMFObject::query(const string &key)
{
    int index = indexOf(key);
    if (index >= 0) {
        AMF0Any *any = value(index);
        if (!any) {
            return NULL;
        }
        return any;
    }

    return NULL;
}

int AMFObject::indexOf(const string &key)
{
    for (unsigned int i = 0; i < values.size(); ++i) {
        Amf0ObjectProperty &p = values.at(i);
        if (p.first == key) {
            return i;
        }
    }

    return -1;
}

void AMFObject::setValue(const string &key, AMF0Any *any)
{
    values.push_back(std::make_pair(key, any));
}

void AMFObject::clear()
{
    for (unsigned int i = 0; i < values.size(); ++i) {
        pair<string, AMF0Any*> &p = values.at(i);
        AMF0Any *any = p.second;
        srs_freep(any);
    }

    values.clear();
}

bool AMFObject::empty()
{
    return values.empty();
}

void AMF0StrictArray::clear()
{
    vector<AMF0Any *>::iterator iter;
    for (iter = values.begin(); iter != values.end(); ++iter) {
        AMF0Any *any = *iter;
        srs_freep(any);
    }

    values.clear();
}

/**********************************************************************/

#define Check_Marker(type) \
    if (!buffer.require(1)) \
    { \
        return false; \
    } \
    int8_t marker = buffer.read_1bytes(); \
    if (marker != type) { \
        return false;\
    }

static bool objectEOF(SrsBuffer &stream, bool &eof)
{
    if (!stream.require(3)) {
        return false;
    }

    int32_t var = stream.read_3bytes();

    eof = (var == AMF0_OBJECT_END);
    if (!eof) {
        stream.skip(-3);
    }

    return true;
}

static bool readString(SrsBuffer &buffer, string &var)
{
    if (!buffer.require(2)) {
        return false;
    }

    int16_t len = buffer.read_2bytes();

    if (!buffer.require(len)) {
        return false;
    }

    var = buffer.read_string(len);
    return true;
}

bool AmfReadString(SrsBuffer &buffer, string &var)
{
    Check_Marker(AMF0_SHORT_STRING);

    return readString(buffer, var);
}

bool AmfReadDouble(SrsBuffer &buffer, double &var)
{
    Check_Marker(AMF0_NUMBER);

    if (!buffer.require(8)) {
        return false;
    }

    int64_t temp = buffer.read_8bytes();
    memcpy(&var, &temp, 8);

    return true;
}

bool AmfReadBoolean(SrsBuffer &buffer, bool &var)
{
    Check_Marker(AMF0_BOOLEAN);

    if (!buffer.require(1)) {
        return false;
    }

    int8_t v = buffer.read_1bytes();
    var = v;

    return true;
}

bool AmfReadNull(SrsBuffer &buffer)
{
    D_UNUSED(buffer);
    Check_Marker(AMF0_NULL);
    return true;
}

bool AmfReadUndefined(SrsBuffer &buffer)
{
    D_UNUSED(buffer);
    Check_Marker(AMF0_UNDEFINED);
    return true;
}

bool AmfReadAny(SrsBuffer &buffer, AMF0Any **var)
{
    bool ret = true;

    if (!buffer.require(1)) {
        return false;
    }

    int8_t marker = buffer.read_1bytes();

    buffer.skip(-1);

    switch (marker) {
    case AMF0_NUMBER:
    {
        AMF0Number *num = new AMF0Number;
        *var = num;
        ret = AmfReadDouble(buffer, num->value);
        break;
    }
    case AMF0_BOOLEAN:
    {
        AMF0Boolean *bl = new AMF0Boolean;
        *var = bl;
        ret = AmfReadBoolean(buffer, bl->value);
        break;
    }
    case AMF0_SHORT_STRING:
    {
        AMF0String *sstr = new AMF0String;
        *var = sstr;
        ret = AmfReadString(buffer, sstr->value);
        break;
    }
    case AMF0_OBJECT:
    {
        AMF0Object *amf0_obj = new AMF0Object;
        *var = amf0_obj;
        ret = AmfReadObject(buffer, *amf0_obj);
        break;
    }
    case AMF0_NULL:
    {
        AMF0Null *nill = new AMF0Null;
        *var = nill;
        ret = AmfReadNull(buffer);
        break;
    }
    case AMF0_UNDEFINED:
    {
        AMF0Undefined *und = new AMF0Undefined;
        *var = und;
        ret = AmfReadUndefined(buffer);
        break;
    }
    case AMF0_ECMA_ARRAY:
    {
        AMF0EcmaArray *ecma_array = new AMF0EcmaArray;
        *var = ecma_array;
        ret = AmfReadEcmaArray(buffer, *ecma_array);
        break;
    }
    case AMF0_STRICT_ARRAY:
    {
        AMF0StrictArray *strict_array = new AMF0StrictArray;
        *var = strict_array;
        ret = AmfReadStrictArray(buffer, *strict_array);
        break;
    }
    case AMF0_DATE:
    {
        AMF0Date *date = new AMF0Date;
        *var = date;
        ret = AmfReadDate(buffer, *date);
        break;
    }
    case AMF0_AMF3_OBJECT:
    {
        // TODO
        return false;
    }
    default:
        return false;
    }

    return ret;
}

bool AmfReadObject(SrsBuffer &buffer, AMF0Object &var)
{
    Check_Marker(AMF0_OBJECT);

    while (!buffer.empty()) {
        bool eof;
        if (!objectEOF(buffer, eof)) {
            var.values.clear();
            return false;
        }

        if (eof) {
            break;
        }

        string key;
        if (!readString(buffer, key)) {
            var.values.clear();
            return false;
        }

        AMF0Any *any = NULL;
        if (!AmfReadAny(buffer, &any)) {
            var.values.clear();
            return false;
        }

        var.setValue(key, any);
    }

    return true;
}

bool AmfReadEcmaArray(SrsBuffer &buffer, AMF0EcmaArray &var)
{
    Check_Marker(AMF0_ECMA_ARRAY);

    if (!buffer.require(4)) {
        return false;
    }

    // read ecma count
    int32_t count = buffer.read_4bytes();

    var.count = count;

    while (!buffer.empty()) {
        bool eof;
        if (!objectEOF(buffer, eof)) {
            var.values.clear();
            return false;
        }

        if (eof) {
            break;
        }

        string key;
        if (!readString(buffer, key)) {
            var.values.clear();
            return false;
        }

        AMF0Any *any = NULL;
        if (!AmfReadAny(buffer, &any)) {
            var.values.clear();
            return false;
        }

        var.setValue(key, any);
    }

    return true;
}

bool AmfReadStrictArray(SrsBuffer &buffer, AMF0StrictArray &var)
{
    Check_Marker(AMF0_STRICT_ARRAY);

    if (!buffer.require(4)) {
        return false;
    }

    // read strict count
    int32_t count = buffer.read_4bytes();

    var.count = count;

    for (int i = 0; i < count && !buffer.empty(); ++i) {
        AMF0Any *any = NULL;
        if (!AmfReadAny(buffer, &any)) {
            var.values.clear();
            return false;
        }

        var.values.push_back(any);
    }

    return true;
}

bool AmfReadDate(SrsBuffer &buffer, AMF0Date &var)
{
    Check_Marker(AMF0_DATE);

    if (!buffer.require(8)) {
        return false;
    }

    int64_t date_value = buffer.read_8bytes();

    var.date_value = date_value;

    if (!buffer.require(2)) {
        return false;
    }

    int16_t time_zone = buffer.read_2bytes();

    var.time_zone = time_zone;

    return true;
}

/**********************************************************************/

static bool writeString(SrsBuffer &buffer, const string &var)
{
    buffer.write_2bytes((int16_t)var.size());
    buffer.write_string(var);
    return true;
}

bool AmfWriteString(SrsBuffer &buffer, const string &var)
{
    buffer.write_1bytes(AMF0_SHORT_STRING);

    return writeString(buffer, var);
}

bool AmfWriteDouble(SrsBuffer &buffer, double value)
{
    buffer.write_1bytes(AMF0_NUMBER);

    int64_t temp = 0x00;
    memcpy(&temp, &value, 8);

    buffer.write_8bytes(temp);
    return true;
}

bool AmfWriteBoolean(SrsBuffer &buffer, bool value)
{
    buffer.write_1bytes(AMF0_BOOLEAN);
    int8_t v = value ? 0x01 : 0x00;
    buffer.write_1bytes(v);
    return true;
}

bool AmfWriteNull(SrsBuffer &buffer)
{
    int8_t v = AMF0_NULL;
    buffer.write_1bytes(v);
    return true;
}

bool AmfWriteUndefined(SrsBuffer &buffer)
{
    int8_t v = AMF0_UNDEFINED;
    buffer.write_1bytes(v);
    return true;
}

bool AmfWriteAny(SrsBuffer &buffer, AMF0Any *any)
{
    int8_t marker = any->type;

    switch (marker) {
    case AMF0_NUMBER:
    {
        AMF0Number *number = dynamic_cast<AMF0Number*>(any);
        return AmfWriteDouble(buffer, number->value);
    }
    case AMF0_BOOLEAN:
    {
        AMF0Boolean *bl = dynamic_cast<AMF0Boolean*>(any);
        return AmfWriteBoolean(buffer, bl->value);
    }
    case AMF0_SHORT_STRING:
    {
        AMF0String *sstr = dynamic_cast<AMF0String*>(any);
        return AmfWriteString(buffer, sstr->value);
    }
    case AMF0_OBJECT:
    {
        AMF0Object *amf0_obj = dynamic_cast<AMF0Object *>(any);
        return AmfWriteObject(buffer, *amf0_obj);
    }
    case AMF0_NULL:
    {
        AMF0Null *nill = dynamic_cast<AMF0Null *>(any);
        D_UNUSED(nill);
        return AmfWriteNull(buffer);
    }
    case AMF0_UNDEFINED:
    {
        AMF0Undefined *und = dynamic_cast<AMF0Undefined *>(any);
        D_UNUSED(und);
        return AmfWriteUndefined(buffer);
    }
    case AMF0_ECMA_ARRAY:
    {
        AMF0EcmaArray *ecma_array = dynamic_cast<AMF0EcmaArray *>(any);
        return AmfWriteEcmaArray(buffer, *ecma_array);
    }
    case AMF0_STRICT_ARRAY:
    {
        AMF0StrictArray *strict_array = dynamic_cast<AMF0StrictArray *>(any);
        return AmfWriteStrictArray(buffer, *strict_array);
    }
    case AMF0_DATE:
    {
        AMF0Date *date = dynamic_cast<AMF0Date*>(any);
        return AmfWriteDate(buffer, *date);
    }
    case AMF0_AMF3_OBJECT:
    {
        // TODO
        return false;
    }
    default:
        return false;
    }

    return true;
}

bool AmfWriteObject(SrsBuffer &buffer, AMF0Object &var)
{
    buffer.write_1bytes(AMF0_OBJECT);

    int size = var.values.size();
    for (int i = 0; i < size; ++i) {
        const string &key = var.key(i);
        AMF0Any *any = var.value(i);

        if (!any) {
            return false;
        }

        if (!writeString(buffer, key)) {
            return false;
        }

        if (!AmfWriteAny(buffer, any)) {
            return false;
        }
    }
    buffer.write_3bytes(AMF0_OBJECT_END);

    return true;
}

bool AmfWriteEcmaArray(SrsBuffer &buffer, AMF0EcmaArray &var)
{
    buffer.write_1bytes(AMF0_ECMA_ARRAY);
    buffer.write_4bytes((uint32_t)var.values.size());

    int size = var.values.size();
    for (int i = 0; i < size; ++i) {
        const string &key = var.key(i);
        AMF0Any *any = var.value(i);

        if (!any) {
            return false;
        }

        if (!writeString(buffer, key)) {
            return false;
        }

        if (!AmfWriteAny(buffer, any)) {
            return false;
        }
    }
    buffer.write_3bytes(AMF0_OBJECT_END);

    return true;
}

bool AmfWriteStrictArray(SrsBuffer &buffer, AMF0StrictArray &var)
{
    buffer.write_1bytes(AMF0_STRICT_ARRAY);
    buffer.write_4bytes(var.count);

    vector<AMF0Any *> &values = var.values;
    for (int i = 0; i < (int)values.size(); ++i) {
        AMF0Any* any = values.at(i);
        if (!any) {
            return false;
        }

        if (!AmfWriteAny(buffer, any)) {
            return false;
        }
    }

    return true;
}

bool AmfWriteDate(SrsBuffer &buffer, AMF0Date &var)
{
    buffer.write_1bytes(AMF0_DATE);
    buffer.write_8bytes(var.date_value);
    buffer.write_2bytes(var.time_zone);
    return true;
}

bool AmfWriteEcmaArray(SrsBuffer &buffer, AMF0Object &var)
{
    buffer.write_1bytes(AMF0_ECMA_ARRAY);
    buffer.write_4bytes((uint32_t)var.values.size());

    int size = var.values.size();
    for (int i = 0; i < size; ++i) {
        const string &key = var.key(i);
        AMF0Any *any = var.value(i);

        if (!any) {
            return false;
        }

        if (!writeString(buffer, key)) {
            return false;
        }

        if (!AmfWriteAny(buffer, any)) {
            return false;
        }
    }
    buffer.write_3bytes(AMF0_OBJECT_END);

    return true;
}

/******************************************************/
int SrsAmf0Size::utf8(string value)
{
    return 2 + value.length();
}

int SrsAmf0Size::str(string value)
{
    return 1 + SrsAmf0Size::utf8(value);
}

int SrsAmf0Size::number()
{
    return 1 + 8;
}

int SrsAmf0Size::date()
{
    return 1 + 8 + 2;
}

int SrsAmf0Size::null()
{
    return 1;
}

int SrsAmf0Size::undefined()
{
    return 1;
}

int SrsAmf0Size::boolean()
{
    return 1 + 1;
}

int SrsAmf0Size::object(AMF0Object *obj)
{
    if (!obj) {
        return 0;
    }

    return obj->total_size();
}

int SrsAmf0Size::object_eof()
{
    return 2 + 1;
}

int SrsAmf0Size::ecma_array(AMF0EcmaArray* arr)
{
    if (!arr) {
        return 0;
    }

    return arr->total_size();
}

int SrsAmf0Size::strict_array(AMF0StrictArray *arr)
{
    if (!arr) {
        return 0;
    }

    return arr->total_size();
}

int SrsAmf0Size::any(AMF0Any *o)
{
    if (!o) {
        return 0;
    }

    return o->total_size();
}

/*********************************************/
int AMF0Number::total_size()
{
    return SrsAmf0Size::number();
}

int AMF0Boolean::total_size()
{
    return SrsAmf0Size::boolean();
}

int AMF0String::total_size()
{
    return SrsAmf0Size::str(value);
}

int AMF0Object::total_size()
{
    int size = 1;

    for (int i = 0; i < (int)values.size(); i++){
        std::string name = key(i);
        AMF0Any* val = value(i);

        size += SrsAmf0Size::utf8(name);
        size += SrsAmf0Size::any(val);
    }

    size += SrsAmf0Size::object_eof();

    return size;
}

int AMF0Null::total_size()
{
    return SrsAmf0Size::null();
}

int AMF0Undefined::total_size()
{
    return SrsAmf0Size::undefined();
}

int AMF0EcmaArray::total_size()
{
    int size = 1 + 4;

    for (int i = 0; i < (int)values.size(); i++){
        std::string name = key(i);
        AMF0Any* val = value(i);

        size += SrsAmf0Size::utf8(name);
        size += SrsAmf0Size::any(val);
    }

    size += SrsAmf0Size::object_eof();

    return size;
}

int AMF0StrictArray::total_size()
{
    int size = 1 + 4;

    for (int i = 0; i < (int)values.size(); i++){
        AMF0Any* any = values[i];
        size += any->total_size();
    }

    return size;
}

int AMF0Date::total_size()
{
    return SrsAmf0Size::date();
}
