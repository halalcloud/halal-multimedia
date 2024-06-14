#include <QCoreApplication>
#include "bbits.h"
//#include "get_bits.cpp"
#include <cassert>
#define ADTS_HEADER_SIZE 7
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    uint8_t *buf = new uint8_t[10];
    size_t size = 10;
    PutBitContext pb;
    init_put_bits(&pb, buf, ADTS_HEADER_SIZE);

    /* adts_fixed_header */
    put_bits(&pb, 12, 0xfff);   /* syncword */
    put_bits(&pb, 1, 0);        /* ID */
    put_bits(&pb, 2, 0);        /* layer */
    put_bits(&pb, 1, 1);        /* protection_absent */
    //        put_bits(&pb, 2, m_adts->objecttype); /* profile_objecttype */
    //        put_bits(&pb, 4, m_adts->sample_rate_index);
    put_bits(&pb, 1, 0);        /* private_bit */
    //put_bits(&pb, 3, m_adts->channel_conf); /* channel_configuration */
    put_bits(&pb, 1, 0);        /* original_copy */
    put_bits(&pb, 1, 0);        /* home */

    /* adts_variable_header */
    put_bits(&pb, 1, 0);        /* copyright_identification_bit */
    put_bits(&pb, 1, 0);        /* copyright_identification_start */
    put_bits(&pb, 13, size); /* aac_frame_length */
    put_bits(&pb, 11, 0x7ff);   /* adts_buffer_fullness */
    put_bits(&pb, 2, 0);        /* number_of_raw_data_blocks_in_frame */
    flush_put_bits(&pb);

    GetBitContext gb;
    init_get_bits(&gb, buf,size);
    assert(0xfff == get_bits(&gb, 12));
get_ue_golomb(&gb);
    return a.exec();
}
