#ifndef GET_PUT_BITS_H
#define GET_PUT_BITS_H

#ifndef BBITS_API
#  ifdef _WIN32
#     if defined(BBITS_BUILD_SHARED) /* build dll */
#         define BBITS_API __declspec(dllexport)
#     elif !defined(BBITS_USE_SHARED) /* use dll */
#         define BBITS_API __declspec(dllimport)
#     else /* static library */
#         define BBITS_API
#     endif
#  else
#include "stdint.h"
#     if __GNUC__ >= 4
#         define BBITS_API __attribute__((visibility("default")))
#     else
#         define BBITS_API
#     endif
#  endif
#endif

#define BBITS_VERSION_MAJOR 1
#define BBITS_VERSION_MINOR 0
#define BBITS_VERSION ((BBITS_VERSION_MAJOR << 16) | BBITS_VERSION_MINOR)
BBITS_API uint32_t bbits_get_version();
BBITS_API int32_t bbits_is_compatiable();
typedef struct PutBitContext {
    uint32_t bit_buf;
    int bit_left;
    uint8_t *buf, *buf_ptr, *buf_end;
    int size_in_bits;
} PutBitContext;

BBITS_API void init_put_bits(PutBitContext *s, uint8_t *buffer,
                          int buffer_size);
BBITS_API void put_bits(PutBitContext *s, int n, unsigned int value);
BBITS_API void flush_put_bits(PutBitContext *s);
BBITS_API int put_bits_count(PutBitContext *s);

 typedef struct GetBitContext {
     const uint8_t *buffer, *buffer_end;
     int index;
     int size_in_bits;
     int size_in_bits_plus8;
 } GetBitContext;

BBITS_API int init_get_bits(GetBitContext *s, const uint8_t *buffer,
                         int bit_size);
BBITS_API unsigned int get_bits_long(GetBitContext *s, int n);
BBITS_API void skip_bits_long(GetBitContext *s, int n);
BBITS_API unsigned int get_bits(GetBitContext *s, int n);
BBITS_API int get_bits_count(const GetBitContext *s);
BBITS_API void skip_bits(GetBitContext *s, int n);
BBITS_API unsigned int get_bits1(GetBitContext *s);
BBITS_API unsigned int show_bits(GetBitContext *s, int n);
BBITS_API unsigned int show_bits_long(GetBitContext *s, int n);
BBITS_API int get_bits_left(GetBitContext *gb);
BBITS_API int get_bits_count(const GetBitContext *s);
BBITS_API int get_ue_golomb(GetBitContext *gb);
BBITS_API unsigned get_ue_golomb_long(GetBitContext *gb);
BBITS_API int get_ue_golomb_31(GetBitContext *gb);
BBITS_API int get_se_golomb(GetBitContext *gb);
BBITS_API int get_se_golomb_long(GetBitContext *gb);
#endif
