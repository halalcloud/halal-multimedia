#ifndef GET_PUT_BITS_H
#define GET_PUT_BITS_H
#ifdef LIB_GET_PUT_BITS
#define EXPORT __attribute__ ((visibility("default")))
#else
#define EXPORT __attribute__ ((visibility("default")))
#endif
typedef struct PutBitContext {
    uint32_t bit_buf;
    int bit_left;
    uint8_t *buf, *buf_ptr, *buf_end;
    int size_in_bits;
} PutBitContext;

EXPORT void init_put_bits(PutBitContext *s, uint8_t *buffer,
                          int buffer_size);
EXPORT void put_bits(PutBitContext *s, int n, unsigned int value);
EXPORT void flush_put_bits(PutBitContext *s);
EXPORT int put_bits_count(PutBitContext *s);

 typedef struct GetBitContext {
     const uint8_t *buffer, *buffer_end;
     int index;
     int size_in_bits;
     int size_in_bits_plus8;
 } GetBitContext;

EXPORT int init_get_bits(GetBitContext *s, const uint8_t *buffer,
                         int bit_size);
EXPORT unsigned int get_bits_long(GetBitContext *s, int n);
EXPORT void skip_bits_long(GetBitContext *s, int n);
EXPORT unsigned int get_bits(GetBitContext *s, int n);
EXPORT int get_bits_count(const GetBitContext *s);
EXPORT void skip_bits(GetBitContext *s, int n);
EXPORT unsigned int get_bits1(GetBitContext *s);
EXPORT unsigned int show_bits(GetBitContext *s, int n);
EXPORT unsigned int show_bits_long(GetBitContext *s, int n);
EXPORT int get_bits_left(GetBitContext *gb);
EXPORT int get_bits_count(const GetBitContext *s);
EXPORT int get_ue_golomb(GetBitContext *gb);
EXPORT unsigned get_ue_golomb_long(GetBitContext *gb);
EXPORT int get_ue_golomb_31(GetBitContext *gb);
EXPORT int get_se_golomb(GetBitContext *gb);
EXPORT int get_se_golomb_long(GetBitContext *gb);
#endif
