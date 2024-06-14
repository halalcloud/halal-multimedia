#include "h264_parser.h"
#include <stdio.h>
#include <string.h>
static const uint8_t trailing_bits[] = { 0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };//9
static uint32_t calc_ceil_log2(uint32_t val)
{
    uint32_t ix, cval;
    ix = 0;
    cval = 1;
    while(ix < 32)
    {
        if(cval >= val)
            return ix;
        cval <<= 1;
        ix++;
    }
    return ix;
}
static void scaling_list(uint32_t ix, uint32_t sizeOfScalingList, GetBitContext *bs)
{
    uint32_t lastScale = 8, nextScale = 8;
    uint32_t jx;
    int deltaScale;
    for(jx = 0; jx < sizeOfScalingList; jx++)
    {
        if(nextScale != 0)
        {
            deltaScale = get_se_golomb(bs);
            nextScale =(lastScale + deltaScale + 256) % 256;
        }
        if(nextScale == 0)
        {
            lastScale = lastScale;
        }
        else
        {
            lastScale = nextScale;
        }
    }
}
/**
 * @brief 解析SPS
 * @param dec
 * @param Buffer 不包含前导０
 * @param Buffer_Size
 */
void h264_parse_sequence_parameter_set(h264_decode_t *dec, uint8_t* Buffer,size_t Buffer_Size)
{
    memset(dec, 0, sizeof(h264_decode_t));
    GetBitContext ourbs;
    init_get_bits(&ourbs, Buffer, Buffer_Size*8);
    GetBitContext *bs = &ourbs;
    uint32_t type = 0;
    uint8_t mark = get_bits1(bs);
    if(0 !=mark) return;
    dec->nal_ref_idc = get_bits(bs,2);//NAL 的优先级
    dec->nal_unit_type = type = get_bits(bs,5);//nal类型
    if(type != 0x7)   {    return;
    }

    uint32_t temp;
    dec->profile = get_bits(bs,8);//第个字节
    temp = get_bits1(bs);
    temp = get_bits1(bs);
    temp = get_bits1(bs);
    temp = get_bits1(bs);
    mark = get_bits(bs, 4);//第个字节
    if (mark != 0) return;
    dec->level = temp = get_bits(bs,8);//第个字节
    temp = get_ue_golomb(bs);
    if(dec->profile == 100 || dec->profile == 110 || dec->profile == 122 || dec->profile == 144)
    {

        dec->chroma_format_idc = temp = get_ue_golomb(bs);
        if(dec->chroma_format_idc == 3)
        {
            dec->residual_colour_transform_flag = get_bits1(bs);
        }
        dec->bit_depth_luma_minus8 = temp =get_ue_golomb(bs);
        dec->bit_depth_chroma_minus8 = temp = get_ue_golomb(bs);
        dec->qpprime_y_zero_transform_bypass_flag = temp = get_bits1(bs);
        dec->seq_scaling_matrix_present_flag = temp = get_bits1(bs);
        if(dec->seq_scaling_matrix_present_flag)
        {
            for(uint32_t ix = 0; ix < 8; ix++)
            {
                temp = get_bits1(bs);
                if(temp)
                {
                    scaling_list(ix, ix < 6 ? 16 : 64, bs);
                }
            }
        }

    }
    dec->log2_max_frame_num_minus4 = temp = get_ue_golomb(bs);
    dec->pic_order_cnt_type = temp = get_ue_golomb(bs);
    if(dec->pic_order_cnt_type == 0)
    {
        dec->log2_max_pic_order_cnt_lsb_minus4 = get_ue_golomb(bs);
    }
    else if(dec->pic_order_cnt_type == 1)
    {
        dec->delta_pic_order_always_zero_flag = get_bits1(bs);
        temp = get_se_golomb(bs);
        temp = get_se_golomb(bs);
        temp = get_ue_golomb(bs);
        for(uint32_t ix = 0; ix < temp; ix++)
        {
            temp = get_se_golomb(bs);
        }
    }
    temp = get_ue_golomb(bs);
    temp = get_bits1(bs);
    uint32_t PicWidthInMbs = get_ue_golomb(bs) + 1;
    uint32_t PicHeightInMapUnits = get_ue_golomb(bs) + 1;
    dec->frame_mbs_only_flag = get_bits1(bs);
    if(!dec->frame_mbs_only_flag)
    {
        temp = get_bits1(bs);
    }
    temp = get_bits1(bs);
    dec->frame_cropping_flag =temp = get_bits1(bs);
    if(temp)
    {
        dec->frame_crop_left_offset = temp = get_ue_golomb(bs);
        dec->frame_crop_right_offset = temp = get_ue_golomb(bs);
        dec->frame_crop_top_offset = temp = get_ue_golomb(bs);
        dec->frame_crop_bottom_offset = temp = get_ue_golomb(bs);
    }
    temp = get_bits1(bs);
    if(temp)
    {
    // h264_vui_parameters(dec, bs);//海康sps可能没有这个东西
    }

    // calc real height width
    uint32_t width = PicWidthInMbs * 16;
    uint32_t height = ( 2 - dec->frame_mbs_only_flag ) * PicHeightInMapUnits * 16;

    if (dec->frame_cropping_flag) {
        unsigned int crop_h = 2*(dec->frame_crop_left_offset + dec->frame_crop_right_offset);
        unsigned int crop_v = 2*(dec->frame_crop_top_offset + dec->frame_crop_bottom_offset)*(2-dec->frame_mbs_only_flag);
        if (crop_h < width) width   -= crop_h;
        if (crop_v < height) height -= crop_v;
    }
    dec->pic_width = width;
    dec->pic_height =height;
}
/**
 * @brief h264_parse_pic_parameter_set
 * @param dec
 * @param bs
 */
void   h264_parse_pic_parameter_set(h264_decode_t *dec, GetBitContext *bs)
{
    uint32_t num_slice_groups, temp, iGroup;
    temp = get_ue_golomb(bs);
    temp = get_ue_golomb(bs);
    temp =  get_bits1(bs);
    dec->pic_order_present_flag = get_bits1(bs);
    num_slice_groups = get_ue_golomb(bs);
    if(num_slice_groups > 0)
    {
        temp = get_ue_golomb(bs);
        if(temp == 0)
        {
            for(iGroup = 0; iGroup <= num_slice_groups; iGroup++)
            {
                temp =  get_ue_golomb(bs);
            }
        }
        else if(temp == 2)
        {
            for (iGroup = 0; iGroup < num_slice_groups; iGroup++)
            {
                temp =  get_ue_golomb(bs);
                temp =  get_ue_golomb(bs);
            }
        }
        else if(temp < 6)                    // 3, 4, 5
        {
            temp =  get_bits1(bs);
            temp =  get_ue_golomb(bs);
        }
        else if(temp == 6)
        {
            temp = get_ue_golomb(bs);
            uint32_t bits = calc_ceil_log2(num_slice_groups + 1);
            for(iGroup = 0; iGroup <= temp; iGroup++)
            {
                temp =  get_bits(bs,bits);
            }
        }
    }
    temp = get_ue_golomb(bs);
    temp = get_ue_golomb(bs);
    temp =  get_bits1(bs);
    temp =  get_bits(bs,2);
    temp = get_se_golomb(bs);
    temp = get_se_golomb(bs);
    temp = get_se_golomb(bs);
    temp =  get_bits1(bs);
    temp =  get_bits1(bs);
    temp =  get_bits1(bs);
    int bits = get_bits_left(bs);
    if(bits == 0)
        return;
    if(bits <= 8)
    {
        uint8_t trail_check = show_bits(bs,bits);
        if(trail_check == trailing_bits[bits])
            return;
    }
    // we have the extensions
    uint8_t transform_8x8_mode_flag = get_bits1(bs);
    temp = get_bits1(bs);
    if(temp)
    {
        uint32_t max_count = 6 +(2 * transform_8x8_mode_flag);
        for(uint32_t ix = 0; ix < max_count; ix++)
        {
            temp = get_bits1(bs);
            if (temp)
            {
                scaling_list(ix, ix < 6 ? 16 : 64, bs);
            }
        }
    }
    temp = get_se_golomb(bs);
}
