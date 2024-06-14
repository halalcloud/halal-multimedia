#include "SrsBuffer.hpp"
#include "RtmpGlobal.hpp"
#include <assert.h>

SrsBuffer::SrsBuffer()
{
    set_value(NULL, 0);
}

SrsBuffer::SrsBuffer(char* b, int nb_b)
{
    set_value(b, nb_b);
}

SrsBuffer::~SrsBuffer()
{
}

void SrsBuffer::set_value(char *b, int nb_b)
{
    p = bytes = b;
    nb_bytes = nb_b;

    // TODO: support both little and big endian.
    assert(srs_is_little_endian());
}

int SrsBuffer::initialize(char *b, int nb)
{
    if (!b) {
        return -1;
    }

    if (nb <= 0) {
        return -2;
    }

    nb_bytes = nb;
    p = bytes = b;

    return 0;
}

char* SrsBuffer::data()
{
    return bytes;
}

int SrsBuffer::size()
{
    return nb_bytes;
}

int SrsBuffer::pos()
{
    return (int)(p - bytes);
}

bool SrsBuffer::empty()
{
    return !bytes || (p >= bytes + nb_bytes);
}

bool SrsBuffer::require(int required_size)
{
    assert(required_size >= 0);

    return required_size <= nb_bytes - (p - bytes);
}

void SrsBuffer::skip(int size)
{
    assert(p);

    p += size;
}

int8_t SrsBuffer::read_1bytes()
{
    assert(require(1));

    return (int8_t)*p++;
}

int16_t SrsBuffer::read_2bytes()
{
    assert(require(2));

    int16_t value;
    char* pp = (char*)&value;
    pp[1] = *p++;
    pp[0] = *p++;

    return value;
}

int32_t SrsBuffer::read_3bytes()
{
    assert(require(3));

    int32_t value = 0x00;
    char* pp = (char*)&value;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;

    return value;
}

int32_t SrsBuffer::read_4bytes()
{
    assert(require(4));

    int32_t value;
    char* pp = (char*)&value;
    pp[3] = *p++;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;

    return value;
}

int64_t SrsBuffer::read_8bytes()
{
    assert(require(8));

    int64_t value;
    char* pp = (char*)&value;
    pp[7] = *p++;
    pp[6] = *p++;
    pp[5] = *p++;
    pp[4] = *p++;
    pp[3] = *p++;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;

    return value;
}

string SrsBuffer::read_string(int len)
{
    assert(require(len));

    std::string value;
    value.append(p, len);

    p += len;

    return value;
}

void SrsBuffer::read_bytes(char* data, int size)
{
    assert(require(size));

    memcpy(data, p, size);

    p += size;
}

void SrsBuffer::write_1bytes(int8_t value)
{
    assert(require(1));

    *p++ = value;
}

void SrsBuffer::write_2bytes(int16_t value)
{
    assert(require(2));

    char* pp = (char*)&value;
    *p++ = pp[1];
    *p++ = pp[0];
}

void SrsBuffer::write_4bytes(int32_t value)
{
    assert(require(4));

    char* pp = (char*)&value;
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void SrsBuffer::write_3bytes(int32_t value)
{
    assert(require(3));

    char* pp = (char*)&value;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void SrsBuffer::write_8bytes(int64_t value)
{
    assert(require(8));

    char* pp = (char*)&value;
    *p++ = pp[7];
    *p++ = pp[6];
    *p++ = pp[5];
    *p++ = pp[4];
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void SrsBuffer::write_string(string value)
{
    assert(require((int)value.length()));

    memcpy(p, value.data(), value.length());
    p += value.length();
}

void SrsBuffer::write_bytes(char* data, int size)
{
    assert(require(size));

    memcpy(p, data, size);
    p += size;
}
