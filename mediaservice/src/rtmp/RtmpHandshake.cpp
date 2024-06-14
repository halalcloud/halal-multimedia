#include "RtmpHandshake.hpp"
#include "RtmpGlobal.hpp"
#include <assert.h>

// for openssl_HMACsha256
#include <openssl/evp.h>
#include <openssl/hmac.h>
// for openssl_generate_key
#include <openssl/dh.h>

using namespace _srs_internal;

static HMAC_CTX *HMAC_CTX_new(void)
{
    HMAC_CTX *ctx = (HMAC_CTX *)malloc(sizeof(*ctx));
    if (ctx != NULL) {
        HMAC_CTX_init(ctx);
    }
    return ctx;
}

static void HMAC_CTX_free(HMAC_CTX *ctx)
{
    if (ctx != NULL) {
        HMAC_CTX_cleanup(ctx);
        free(ctx);
    }
}

static void DH_get0_key(const DH *dh, const BIGNUM **pub_key, const BIGNUM **priv_key)
{
    if (pub_key != NULL) {
        *pub_key = dh->pub_key;
    }
    if (priv_key != NULL) {
        *priv_key = dh->priv_key;
    }
}

static int DH_set0_pqg(DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
    /* If the fields p and g in d are NULL, the corresponding input
     * parameters MUST be non-NULL.  q may remain NULL.
     */
    if ((dh->p == NULL && p == NULL)
        || (dh->g == NULL && g == NULL))
        return 0;

    if (p != NULL) {
        BN_free(dh->p);
        dh->p = p;
    }
    if (q != NULL) {
        BN_free(dh->q);
        dh->q = q;
    }
    if (g != NULL) {
        BN_free(dh->g);
        dh->g = g;
    }

    if (q != NULL) {
        dh->length = BN_num_bits(q);
    }

    return 1;
}

static int DH_set_length(DH *dh, long length)
{
    dh->length = length;
    return 1;
}

namespace _srs_internal
{

// 68bytes FMS key which is used to sign the sever packet.
uint8_t SrsGenuineFMSKey[] = {
    0x47, 0x65, 0x6e, 0x75, 0x69, 0x6e, 0x65, 0x20,
    0x41, 0x64, 0x6f, 0x62, 0x65, 0x20, 0x46, 0x6c,
    0x61, 0x73, 0x68, 0x20, 0x4d, 0x65, 0x64, 0x69,
    0x61, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
    0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Media Server 001
    0xf0, 0xee, 0xc2, 0x4a, 0x80, 0x68, 0xbe, 0xe8,
    0x2e, 0x00, 0xd0, 0xd1, 0x02, 0x9e, 0x7e, 0x57,
    0x6e, 0xec, 0x5d, 0x2d, 0x29, 0x80, 0x6f, 0xab,
    0x93, 0xb8, 0xe6, 0x36, 0xcf, 0xeb, 0x31, 0xae
}; // 68

// 62bytes FP key which is used to sign the client packet.
uint8_t SrsGenuineFPKey[] = {
    0x47, 0x65, 0x6E, 0x75, 0x69, 0x6E, 0x65, 0x20,
    0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x46, 0x6C,
    0x61, 0x73, 0x68, 0x20, 0x50, 0x6C, 0x61, 0x79,
    0x65, 0x72, 0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Player 001
    0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8,
    0x2E, 0x00, 0xD0, 0xD1, 0x02, 0x9E, 0x7E, 0x57,
    0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
    0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
}; // 62

int do_openssl_HMACsha256(HMAC_CTX* ctx, const void* data, int data_size, void* digest, unsigned int* digest_size)
{
    if (HMAC_Update(ctx, (unsigned char *) data, data_size) < 0) {
        return -1;
    }

    if (HMAC_Final(ctx, (unsigned char *) digest, digest_size) < 0) {
        return -2;
    }

    return 0;
}

/**
 * sha256 digest algorithm.
 * @param key the sha256 key, NULL to use EVP_Digest, for instance,
 *       hashlib.sha256(data).digest().
 */
int openssl_HMACsha256(const void* key, int key_size, const void* data, int data_size, void* digest)
{
    int ret = 0;

    unsigned int digest_size = 0;

    unsigned char* temp_key = (unsigned char*)key;
    unsigned char* temp_digest = (unsigned char*)digest;

    if (key == NULL) {
        // use data to digest.
        // @see ./crypto/sha/sha256t.c
        // @see ./crypto/evp/digest.c
        if (EVP_Digest(data, data_size, temp_digest, &digest_size, EVP_sha256(), NULL) < 0) {
            return -1;
        }
    } else {
        // use key-data to digest.
        HMAC_CTX *ctx = HMAC_CTX_new();
        if (ctx == NULL) {
            return -2;
        }
        // @remark, if no key, use EVP_Digest to digest,
        // for instance, in python, hashlib.sha256(data).digest().
        if (HMAC_Init_ex(ctx, temp_key, key_size, EVP_sha256(), NULL) < 0) {
            HMAC_CTX_free(ctx);
            return -3;
        }

        ret = do_openssl_HMACsha256(ctx, data, data_size, temp_digest, &digest_size);
        HMAC_CTX_free(ctx);

        if (ret != 0) {
            return -4;
        }
    }

    if (digest_size != 32) {
        return -5;
    }

    return ret;
}

#define RFC2409_PRIME_1024 \
"FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
"29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
"EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
"E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
"EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE65381" \
"FFFFFFFFFFFFFFFF"

SrsDH::SrsDH()
{
    pdh = NULL;
}

SrsDH::~SrsDH()
{
    close();
}

void SrsDH::close()
{
    if (pdh != NULL) {
        DH_free(pdh);
        pdh = NULL;
    }
}

int SrsDH::initialize(bool ensure_128bytes_public_key)
{
    for (;;) {
        if (do_initialize() != 0) {
            return -1;
        }

        if (ensure_128bytes_public_key) {
            const BIGNUM *pub_key = NULL;
            DH_get0_key(pdh, &pub_key, NULL);
            int32_t key_size = BN_num_bytes(pub_key);
            if (key_size != 128) {
                continue;
            }
        }

        break;
    }

    return 0;
}

int SrsDH::copy_public_key(char* pkey, int32_t& pkey_size)
{
    // copy public key to bytes.
    // sometimes, the key_size is 127, seems ok.
    const BIGNUM *pub_key = NULL;
    DH_get0_key(pdh, &pub_key, NULL);
    int32_t key_size = BN_num_bytes(pub_key);
    assert(key_size > 0);

    // maybe the key_size is 127, but dh will write all 128bytes pkey,
    // so, donot need to set/initialize the pkey.
    // @see https://github.com/ossrs/srs/issues/165
    key_size = BN_bn2bin(pub_key, (unsigned char*)pkey);
    assert(key_size > 0);

    // output the size of public key.
    // @see https://github.com/ossrs/srs/issues/165
    assert(key_size <= pkey_size);
    pkey_size = key_size;

    return 0;
}

int SrsDH::copy_shared_key(const char* ppkey, int32_t ppkey_size, char* skey, int32_t& skey_size)
{
    int ret = 0;

    BIGNUM* ppk = NULL;
    if ((ppk = BN_bin2bn((const unsigned char*)ppkey, ppkey_size, 0)) == NULL) {
        return -1;
    }

    // if failed, donot return, do cleanup, @see ./test/dhtest.c:168
    // maybe the key_size is 127, but dh will write all 128bytes skey,
    // so, donot need to set/initialize the skey.
    // @see https://github.com/ossrs/srs/issues/165
    int32_t key_size = DH_compute_key((unsigned char*)skey, ppk, pdh);

    if (key_size < ppkey_size) {
    }

    if (key_size < 0 || key_size > skey_size) {
        ret = -2;
    } else {
        skey_size = key_size;
    }

    if (ppk) {
        BN_free(ppk);
    }

    return ret;
}

int SrsDH::do_initialize()
{
    int32_t bits_count = 1024;

    close();

    //1. Create the DH
    if ((pdh = DH_new()) == NULL) {
        return -1;
    }

    //2. Create his internal p and g
    BIGNUM *p, *g;
    if ((p = BN_new()) == NULL) {
        return -2;
    }
    if ((g = BN_new()) == NULL) {
        BN_free(p);
        return -3;
    }
    DH_set0_pqg(pdh, p, NULL, g);

    //3. initialize p and g, @see ./test/ectest.c:260
    if (!BN_hex2bn(&p, RFC2409_PRIME_1024)) {
        return -4;
    }
    // @see ./test/bntest.c:1764
    if (!BN_set_word(g, 2)) {
        return -5;
    }

    // 4. Set the key length
    DH_set_length(pdh, bits_count);

    // 5. Generate private and public key
    // @see ./test/dhtest.c:152
    if (!DH_generate_key(pdh)) {
        return -6;
    }

    return 0;
}

key_block::key_block()
{
    offset = (int32_t)rand();
    random0 = NULL;
    random1 = NULL;

    int valid_offset = calc_valid_offset();
    assert(valid_offset >= 0);

    random0_size = valid_offset;
    if (random0_size > 0) {
        random0 = new char[random0_size];
        srs_random_generate(random0, random0_size);
    }

    srs_random_generate(key, sizeof(key));

    random1_size = 764 - valid_offset - 128 - 4;
    if (random1_size > 0) {
        random1 = new char[random1_size];
        srs_random_generate(random1, random1_size);
    }
}

key_block::~key_block()
{
    srs_freepa(random0);
    srs_freepa(random1);
}

int key_block::parse(SrsBuffer* stream)
{
    // the key must be 764 bytes.
    assert(stream->require(764));

    // read the last offset first, 760-763
    stream->skip(764 - sizeof(int32_t));
    offset = stream->read_4bytes();

    // reset stream to read others.
    stream->skip(-764);

    int valid_offset = calc_valid_offset();
    assert(valid_offset >= 0);

    random0_size = valid_offset;
    if (random0_size > 0) {
        srs_freepa(random0);
        random0 = new char[random0_size];
        stream->read_bytes(random0, random0_size);
    }

    stream->read_bytes(key, 128);

    random1_size = 764 - valid_offset - 128 - 4;
    if (random1_size > 0) {
        srs_freepa(random1);
        random1 = new char[random1_size];
        stream->read_bytes(random1, random1_size);
    }

    return 0;
}

int key_block::calc_valid_offset()
{
    int max_offset_size = 764 - 128 - 4;

    int valid_offset = 0;
    uint8_t* pp = (uint8_t*)&offset;
    valid_offset += *pp++;
    valid_offset += *pp++;
    valid_offset += *pp++;
    valid_offset += *pp++;

    return valid_offset % max_offset_size;
}

digest_block::digest_block()
{
    offset = (int32_t)rand();
    random0 = NULL;
    random1 = NULL;

    int valid_offset = calc_valid_offset();
    assert(valid_offset >= 0);

    random0_size = valid_offset;
    if (random0_size > 0) {
        random0 = new char[random0_size];
        srs_random_generate(random0, random0_size);
    }

    srs_random_generate(digest, sizeof(digest));

    random1_size = 764 - 4 - valid_offset - 32;
    if (random1_size > 0) {
        random1 = new char[random1_size];
        srs_random_generate(random1, random1_size);
    }
}

digest_block::~digest_block()
{
    srs_freepa(random0);
    srs_freepa(random1);
}

int digest_block::parse(SrsBuffer* stream)
{
    // the digest must be 764 bytes.
    assert(stream->require(764));

    offset = stream->read_4bytes();

    int valid_offset = calc_valid_offset();
    assert(valid_offset >= 0);

    random0_size = valid_offset;
    if (random0_size > 0) {
        srs_freepa(random0);
        random0 = new char[random0_size];
        stream->read_bytes(random0, random0_size);
    }

    stream->read_bytes(digest, 32);

    random1_size = 764 - 4 - valid_offset - 32;
    if (random1_size > 0) {
        srs_freepa(random1);
        random1 = new char[random1_size];
        stream->read_bytes(random1, random1_size);
    }

    return 0;
}

int digest_block::calc_valid_offset()
{
    int max_offset_size = 764 - 32 - 4;

    int valid_offset = 0;
    uint8_t* pp = (uint8_t*)&offset;
    valid_offset += *pp++;
    valid_offset += *pp++;
    valid_offset += *pp++;
    valid_offset += *pp++;

    return valid_offset % max_offset_size;
}

c1s1_strategy::c1s1_strategy()
{
}

c1s1_strategy::~c1s1_strategy()
{
}

char* c1s1_strategy::get_digest()
{
    return digest.digest;
}

char* c1s1_strategy::get_key()
{
    return key.key;
}

int c1s1_strategy::dump(c1s1* owner, char* _c1s1, int size)
{
    assert(size == 1536);
    return copy_to(owner, _c1s1, size, true);
}

int c1s1_strategy::c1_create(c1s1* owner)
{
    // generate digest
    char* c1_digest = NULL;

    if (calc_c1_digest(owner, c1_digest) != 0) {
        return -1;
    }

    assert(c1_digest != NULL);
    unique_ptr<char> buf(c1_digest);

    memcpy(digest.digest, c1_digest, 32);

    return 0;
}

int c1s1_strategy::c1_validate_digest(c1s1* owner, bool& is_valid)
{
    char* c1_digest = NULL;

    if (calc_c1_digest(owner, c1_digest) != 0) {
        return -1;
    }

    assert(c1_digest != NULL);
    unique_ptr<char> buf(c1_digest);

    is_valid = srs_bytes_equals(digest.digest, c1_digest, 32);

    return 0;
}

int c1s1_strategy::s1_create(c1s1* owner, c1s1* c1)
{
    SrsDH dh;

    // ensure generate 128bytes public key.
    if (dh.initialize(true) != 0) {
        return -1;
    }

    // directly generate the public key.
    // @see: https://github.com/ossrs/srs/issues/148
    int pkey_size = 128;
    if (dh.copy_shared_key(c1->get_key(), 128, key.key, pkey_size) != 0) {
        return -2;
    }

    // although the public key is always 128bytes, but the share key maybe not.
    // we just ignore the actual key size, but if need to use the key, must use the actual size.
    // TODO: FIXME: use the actual key size.
    //srs_assert(pkey_size == 128);

    char* s1_digest = NULL;
    if (calc_s1_digest(owner, s1_digest) != 0) {
        return -3;
    }

    assert(s1_digest != NULL);
    unique_ptr<char> buf(s1_digest);

    memcpy(digest.digest, s1_digest, 32);

    return 0;
}

int c1s1_strategy::s1_validate_digest(c1s1* owner, bool& is_valid)
{
    char* s1_digest = NULL;

    if (calc_s1_digest(owner, s1_digest) != 0) {
        return -1;
    }

    assert(s1_digest != NULL);
    unique_ptr<char> buf(s1_digest);

    is_valid = srs_bytes_equals(digest.digest, s1_digest, 32);

    return 0;
}

int c1s1_strategy::calc_c1_digest(c1s1* owner, char*& c1_digest)
{
    /**
     * c1s1 is splited by digest:
     *     c1s1-part1: n bytes (time, version, key and digest-part1).
     *     digest-data: 32bytes
     *     c1s1-part2: (1536-n-32)bytes (digest-part2)
     * @return a new allocated bytes, user must free it.
     */
    char* c1s1_joined_bytes = new char[1536 -32];
    unique_ptr<char> buf(c1s1_joined_bytes);

    if (copy_to(owner, c1s1_joined_bytes, 1536 - 32, false) != 0) {
        return -1;
    }

    c1_digest = new char[SRS_OpensslHashSize];
    if (openssl_HMACsha256(SrsGenuineFPKey, 30, c1s1_joined_bytes, 1536 - 32, c1_digest) != 0) {
        srs_freepa(c1_digest);
        return -2;
    }

    return 0;
}

int c1s1_strategy::calc_s1_digest(c1s1* owner, char*& s1_digest)
{
    /**
     * c1s1 is splited by digest:
     *     c1s1-part1: n bytes (time, version, key and digest-part1).
     *     digest-data: 32bytes
     *     c1s1-part2: (1536-n-32)bytes (digest-part2)
     * @return a new allocated bytes, user must free it.
     */
    char* c1s1_joined_bytes = new char[1536 -32];
    unique_ptr<char> buf(c1s1_joined_bytes);

    if (copy_to(owner, c1s1_joined_bytes, 1536 - 32, false) != 0) {
        return -1;
    }

    s1_digest = new char[SRS_OpensslHashSize];
    if (openssl_HMACsha256(SrsGenuineFMSKey, 36, c1s1_joined_bytes, 1536 - 32, s1_digest) != 0) {
        srs_freepa(s1_digest);
        return -2;
    }

    return 0;
}

void c1s1_strategy::copy_time_version(SrsBuffer* stream, c1s1* owner)
{
    assert(stream->require(8));

    // 4bytes time
    stream->write_4bytes(owner->time);

    // 4bytes version
    stream->write_4bytes(owner->version);
}
void c1s1_strategy::copy_key(SrsBuffer* stream)
{
    assert(key.random0_size >= 0);
    assert(key.random1_size >= 0);

    int total = key.random0_size + 128 + key.random1_size + 4;
    assert(stream->require(total));

    // 764bytes key block
    if (key.random0_size > 0) {
        stream->write_bytes(key.random0, key.random0_size);
    }

    stream->write_bytes(key.key, 128);

    if (key.random1_size > 0) {
        stream->write_bytes(key.random1, key.random1_size);
    }

    stream->write_4bytes(key.offset);
}
void c1s1_strategy::copy_digest(SrsBuffer* stream, bool with_digest)
{
    assert(key.random0_size >= 0);
    assert(key.random1_size >= 0);

    int total = 4 + digest.random0_size + digest.random1_size;
    if (with_digest) {
        total += 32;
    }
    assert(stream->require(total));

    // 732bytes digest block without the 32bytes digest-data
    // nbytes digest block part1
    stream->write_4bytes(digest.offset);

    // digest random padding.
    if (digest.random0_size > 0) {
        stream->write_bytes(digest.random0, digest.random0_size);
    }

    // digest
    if (with_digest) {
        stream->write_bytes(digest.digest, 32);
    }

    // nbytes digest block part2
    if (digest.random1_size > 0) {
        stream->write_bytes(digest.random1, digest.random1_size);
    }
}

c1s1_strategy_schema0::c1s1_strategy_schema0()
{
}

c1s1_strategy_schema0::~c1s1_strategy_schema0()
{
}

srs_schema_type c1s1_strategy_schema0::schema()
{
    return srs_schema0;
}

int c1s1_strategy_schema0::parse(char* _c1s1, int size)
{
    assert(size == 1536);

    SrsBuffer stream;

    if (stream.initialize(_c1s1 + 8, 764) != 0) {
        return -1;
    }

    if (key.parse(&stream) != 0) {
        return -2;
    }

    if (stream.initialize(_c1s1 + 8 + 764, 764) != 0) {
        return -3;
    }

    if (digest.parse(&stream) != 0) {
        return -4;
    }

    return 0;
}

int c1s1_strategy_schema0::copy_to(c1s1* owner, char* bytes, int size, bool with_digest)
{
    if (with_digest) {
        assert(size == 1536);
    } else {
        assert(size == 1504);
    }

    SrsBuffer stream;

    if (stream.initialize(bytes, size) != 0) {
        return -1;
    }

    copy_time_version(&stream, owner);
    copy_key(&stream);
    copy_digest(&stream, with_digest);

    assert(stream.empty());

    return 0;
}

c1s1_strategy_schema1::c1s1_strategy_schema1()
{
}

c1s1_strategy_schema1::~c1s1_strategy_schema1()
{
}

srs_schema_type c1s1_strategy_schema1::schema()
{
    return srs_schema1;
}

int c1s1_strategy_schema1::parse(char* _c1s1, int size)
{
    assert(size == 1536);

    SrsBuffer stream;

    if (stream.initialize(_c1s1 + 8, 764) != 0) {
        return -1;
    }

    if (digest.parse(&stream) != 0) {
        return -2;
    }

    if (stream.initialize(_c1s1 + 8 + 764, 764) != 0) {
        return -3;
    }

    if (key.parse(&stream) != 0) {
        return -4;
    }

    return 0;
}

int c1s1_strategy_schema1::copy_to(c1s1* owner, char* bytes, int size, bool with_digest)
{
    if (with_digest) {
        assert(size == 1536);
    } else {
        assert(size == 1504);
    }

    SrsBuffer stream;

    if (stream.initialize(bytes, size) != 0) {
        return -1;
    }

    copy_time_version(&stream, owner);
    copy_digest(&stream, with_digest);
    copy_key(&stream);

    assert(stream.empty());

    return 0;
}

c1s1::c1s1()
{
    payload = NULL;
}
c1s1::~c1s1()
{
    srs_freep(payload);
}

srs_schema_type c1s1::schema()
{
    assert(payload != NULL);
    return payload->schema();
}

char* c1s1::get_digest()
{
    assert(payload != NULL);
    return payload->get_digest();
}

char* c1s1::get_key()
{
    assert(payload != NULL);
    return payload->get_key();
}

int c1s1::dump(char* _c1s1, int size)
{
    assert(size == 1536);
    assert(payload != NULL);
    return payload->dump(this, _c1s1, size);
}

int c1s1::parse(char* _c1s1, int size, srs_schema_type schema)
{
    assert(size == 1536);

    if (schema != srs_schema0 && schema != srs_schema1) {
        return -1;
    }

    SrsBuffer stream;

    if (stream.initialize(_c1s1, size) != 0) {
        return -2;
    }

    time = stream.read_4bytes();
    version = stream.read_4bytes(); // client c1 version

    srs_freep(payload);
    if (schema == srs_schema0) {
        payload = new c1s1_strategy_schema0();
    } else {
        payload = new c1s1_strategy_schema1();
    }

    return payload->parse(_c1s1, size);
}

int c1s1::c1_create(srs_schema_type schema)
{
    if (schema != srs_schema0 && schema != srs_schema1) {
        return -1;
    }

    // client c1 time and version
    time = (int32_t)::time(NULL);
    version = 0x80000702; // client c1 version

    // generate signature by schema
    srs_freep(payload);
    if (schema == srs_schema0) {
        payload = new c1s1_strategy_schema0();
    } else {
        payload = new c1s1_strategy_schema1();
    }

    return payload->c1_create(this);
}

int c1s1::c1_validate_digest(bool& is_valid)
{
    is_valid = false;
    assert(payload);
    return payload->c1_validate_digest(this, is_valid);
}

int c1s1::s1_create(c1s1* c1)
{
    if (c1->schema() != srs_schema0 && c1->schema() != srs_schema1) {
        return -1;
    }

    time = ::time(NULL);
    version = 0x01000504; // server s1 version

    srs_freep(payload);
    if (c1->schema() == srs_schema0) {
        payload = new c1s1_strategy_schema0();
    } else {
        payload = new c1s1_strategy_schema1();
    }

    return payload->s1_create(this, c1);
}

int c1s1::s1_validate_digest(bool& is_valid)
{
    is_valid = false;
    assert(payload);
    return payload->s1_validate_digest(this, is_valid);
}

c2s2::c2s2()
{
    srs_random_generate(random, 1504);
    srs_random_generate(digest, 32);
}

c2s2::~c2s2()
{
}

int c2s2::dump(char* _c2s2, int size)
{
    assert(size == 1536);

    memcpy(_c2s2, random, 1504);
    memcpy(_c2s2 + 1504, digest, 32);

    return 0;
}

int c2s2::parse(char* _c2s2, int size)
{
    assert(size == 1536);

    memcpy(random, _c2s2, 1504);
    memcpy(digest, _c2s2 + 1504, 32);

    return 0;
}

int c2s2::c2_create(c1s1* s1)
{
    char temp_key[SRS_OpensslHashSize];
    if (openssl_HMACsha256(SrsGenuineFPKey, 62, s1->get_digest(), 32, temp_key) != 0) {
        return -1;
    }

    char _digest[SRS_OpensslHashSize];
    if (openssl_HMACsha256(temp_key, 32, random, 1504, _digest) != 0) {
        return -2;
    }

    memcpy(digest, _digest, 32);

    return 0;
}

int c2s2::c2_validate(c1s1* s1, bool& is_valid)
{
    is_valid = false;

    char temp_key[SRS_OpensslHashSize];
    if (openssl_HMACsha256(SrsGenuineFPKey, 62, s1->get_digest(), 32, temp_key) != 0) {
        return -1;
    }

    char _digest[SRS_OpensslHashSize];
    if (openssl_HMACsha256(temp_key, 32, random, 1504, _digest) != 0) {
        return -2;
    }

    is_valid = srs_bytes_equals(digest, _digest, 32);

    return 0;
}

int c2s2::s2_create(c1s1* c1)
{
    char temp_key[SRS_OpensslHashSize];
    if (openssl_HMACsha256(SrsGenuineFMSKey, 68, c1->get_digest(), 32, temp_key) != 0) {
        return -1;
    }

    char _digest[SRS_OpensslHashSize];
    if (openssl_HMACsha256(temp_key, 32, random, 1504, _digest) != 0) {
        return -2;
    }

    memcpy(digest, _digest, 32);

    return 0;
}

int c2s2::s2_validate(c1s1* c1, bool& is_valid)
{
    is_valid = false;

    char temp_key[SRS_OpensslHashSize];
    if (openssl_HMACsha256(SrsGenuineFMSKey, 68, c1->get_digest(), 32, temp_key) != 0) {
        return -1;
    }

    char _digest[SRS_OpensslHashSize];
    if (openssl_HMACsha256(temp_key, 32, random, 1504, _digest) != 0) {
        return -2;
    }

    is_valid = srs_bytes_equals(digest, _digest, 32);

    return 0;
}

}

/******************************************************************/

RtmpHandshake::RtmpHandshake(IStream *socket)
    : m_socket(socket)
{
    c0c1 = s0s1s2 = c2 = NULL;
    m_c1 = NULL;
    m_type = ComplexC0C1;
}

RtmpHandshake::~RtmpHandshake()
{
    srs_freepa(c0c1);
    srs_freepa(s0s1s2);
    srs_freepa(c2);
    srs_freep(m_c1);
}

int RtmpHandshake::handshake_with_client()
{
    HRESULT hr = S_OK;

    while (true) {
        switch (m_type) {
        case ComplexC0C1:
            hr = parse_complex_c0c1();
            break;
        case ComplexS0S1S2:
            hr = parse_complex_s0s1s2();
            break;
        case ComplexC2:
            hr = parse_complex_c2();
            break;
        case SimpleC0C1:
            hr = parse_simple_c0c1();
            break;
        case SimpleS0S1S2:
            hr = parse_simple_s0s1s2();
            break;
        case SimpleC2:
            hr = parse_simple_c2();
            break;
        default:
            break;
        }

        if (hr < 0) {
            break;
        }

        if (m_type == Completed) {
            break;
        }
    }

    return hr;
}

int RtmpHandshake::handshake_with_server()
{
    HRESULT hr = S_OK;

    while (true) {
        switch (m_type) {
        case ComplexC0C1:
            hr = process_complex_c0c1();
            break;
        case ComplexS0S1S2:
            hr = process_complex_s0s1s2();
            break;
        case ComplexC2:
            hr = process_complex_c2();
            break;
        case SimpleC0C1:
            hr = process_simple_c0c1();
            break;
        case SimpleS0S1S2:
            hr = process_simple_s0s1s2();
            break;
        case SimpleC2:
            hr = process_simple_c2();
            break;
        default:
            break;
        }

        if (hr < 0) {
            break;
        }

        if (m_type == Completed) {
            break;
        }
    }

    return hr;
}

bool RtmpHandshake::completed()
{
    return m_type == Completed;
}

void RtmpHandshake::set_handshake_type(bool complex)
{
    if (complex) {
        m_type = ComplexC0C1;
    } else {
        m_type = SimpleC0C1;
    }
}

int RtmpHandshake::parse_complex_c0c1()
{
    HRESULT hr = S_OK;

    if (!c0c1) {
        c0c1 = new char[1537];
    }

    JIF(m_socket->Read(c0c1, 1537));

    c1s1 c1;
    JIF(c1.parse(c0c1 + 1, 1536, srs_schema0));

    bool is_valid = false;
    if (c1.c1_validate_digest(is_valid) != 0 || !is_valid) {
        JIF(c1.parse(c0c1 + 1, 1536, srs_schema1));

        if (c1.c1_validate_digest(is_valid) != 0 || !is_valid) {
            m_type = SimpleC0C1;
            return 1;
        }
    }

    JIF(s1.s1_create(&c1));

    if (s1.s1_validate_digest(is_valid) != 0 || !is_valid) {
        m_type = SimpleC0C1;
        return 2;
    }

    JIF(s2.s2_create(&c1));

    if (s2.s2_validate(&c1, is_valid) != 0 || !is_valid) {
        m_type = SimpleC0C1;
        return 3;
    }

    m_type = ComplexS0S1S2;

    return hr;
}

int RtmpHandshake::parse_complex_s0s1s2()
{
    HRESULT hr = S_OK;

    if (!m_socket->CanWrite()) {
        return hr;
    }

    JIF(create_s0s1s2());

    JIF(s1.dump(s0s1s2 + 1, 1536));
    JIF(s2.dump(s0s1s2 + 1537, 1536));

    m_type = ComplexC2;

    JIF(m_socket->Write(s0s1s2, 3073));

    return hr;
}

int RtmpHandshake::parse_complex_c2()
{
    HRESULT hr = S_OK;

    if (!c2) {
        c2 = new char[1536];
    }

    JIF(m_socket->Read(c2, 1536));

    c2s2 _c2;
    JIF(_c2.parse(c2, 1536));

    m_type = Completed;

    return hr;
}

int RtmpHandshake::parse_simple_c0c1()
{
    HRESULT hr = S_OK;

    if (!c0c1) {
        c0c1 = new char[1537];
    }

    if (c0c1[0] != 0x03) {
        return -1;
    }

    m_type = SimpleS0S1S2;

    return hr;
}

int RtmpHandshake::parse_simple_s0s1s2()
{
    HRESULT hr = S_OK;

    if (!m_socket->CanWrite()) {
        return hr;
    }

    JIF(create_s0s1s2(c0c1 + 1));

    m_type = SimpleC2;

    JIF(m_socket->Write(s0s1s2, 3073));

    return hr;
}

int RtmpHandshake::parse_simple_c2()
{
    HRESULT hr = S_OK;

    if (!c2) {
        c2 = new char[1536];
    }

    JIF(m_socket->Read(c2, 1536));

    m_type = Completed;

    return hr;
}

int RtmpHandshake::create_s0s1s2(const char *c1)
{
    if (s0s1s2) {
        return 0;
    }

    s0s1s2 = new char[3073];
    srs_random_generate(s0s1s2, 3073);

    // plain text required.
    SrsBuffer stream;
    if (stream.initialize(s0s1s2, 9) != 0) {
        return -1;
    }
    stream.write_1bytes(0x03);
    stream.write_4bytes((int32_t)::time(NULL));
    // s1 time2 copy from c1
    if (c0c1) {
        stream.write_bytes(c0c1 + 1, 4);
    }

    if (c1) {
        memcpy(s0s1s2 + 1537, c1, 1536);
    }

    return 0;
}

int RtmpHandshake::create_c0c1()
{
    if (c0c1) {
        return 0;
    }

    c0c1 = new char[1537];
    srs_random_generate(c0c1, 1537);

    char buf[9];
    SrsBuffer stream;
    if (stream.initialize(buf, 9) != 0) {
        return -1;
    }

    stream.write_1bytes(0x03);
    stream.write_4bytes((int32_t)::time(NULL));
    stream.write_4bytes(0x00);

    memcpy(c0c1, buf, 9);

    return 0;
}

int RtmpHandshake::create_c2()
{
    if (c2) {
        return 0;
    }

    c2 = new char[1536];

    srs_random_generate(c2, 1536);

    char buf[8];
    SrsBuffer stream;
    if (stream.initialize(buf, 8) != 0) {
        return -1;
    }

    stream.write_4bytes((int32_t)::time(NULL));
    // c2 time2 copy from s1
    if (s0s1s2) {
        stream.write_bytes(s0s1s2 + 1, 4);
    }

    memcpy(c2, buf, 8);

    return 0;
}

int RtmpHandshake::process_complex_c0c1()
{
    HRESULT hr = S_OK;

    if (!m_socket->CanWrite()) {
        return hr;
    }

    JIF(create_c0c1());

    srs_freep(m_c1);
    m_c1 = new c1s1();

    JIF(m_c1->c1_create(srs_schema1));
    JIF(m_c1->dump(c0c1 + 1, 1536));

    // verify c1
    bool is_valid;
    if (m_c1->c1_validate_digest(is_valid) != 0 || !is_valid) {
        m_type = SimpleC0C1;
        return 1;
    }

    m_type = ComplexS0S1S2;

    JIF(m_socket->Write(c0c1, 1537));

    return hr;
}

int RtmpHandshake::process_complex_s0s1s2()
{
    HRESULT hr = S_OK;

    if (!s0s1s2) {
        s0s1s2 = new char[3073];
    }

    JIF(m_socket->Read(s0s1s2, 3073));

    // plain text required.
    if (s0s1s2[0] != 0x03) {
        return -1;
    }

    // verify s1s2
    JIF(s1.parse(s0s1s2 + 1, 1536, m_c1->schema()));

    m_type = ComplexC2;

    return hr;
}

int RtmpHandshake::process_complex_c2()
{
    HRESULT hr = S_OK;

    if (!m_socket->CanWrite()) {
        return hr;
    }

    JIF(create_c2());

    c2s2 cs;
    JIF(cs.c2_create(&s1));
    JIF(cs.dump(c2, 1536));

    m_type = Completed;

    JIF(m_socket->Write(c2, 1536));

    return hr;
}

int RtmpHandshake::process_simple_c0c1()
{
    HRESULT hr = S_OK;

    if (!m_socket->CanWrite()) {
        return hr;
    }

    JIF(create_c0c1());

    m_type = SimpleS0S1S2;

    JIF(m_socket->Write(c0c1, 1537));

    return hr;
}

int RtmpHandshake::process_simple_s0s1s2()
{
    HRESULT hr = S_OK;

    if (!s0s1s2) {
        s0s1s2 = new char[3073];
    }

    JIF(m_socket->Read(s0s1s2, 3073));

    if (s0s1s2[0] != 0x03) {
        return -1;
    }

    m_type = SimpleC2;

    return hr;
}

int RtmpHandshake::process_simple_c2()
{
    HRESULT hr = S_OK;

    if (!m_socket->CanWrite()) {
        return hr;
    }

    JIF(create_c2());

    m_type = Completed;

    JIF(m_socket->Write(c2, 1536));

    return hr;
}

