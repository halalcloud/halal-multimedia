#ifndef WZD_MPEG2_H
#define WZD_MPEG2_H


#include "base.h"
#include <vector>
using namespace std;
#include <list>
namespace wzd {

/**
 * PSI表各Setion的基类
 */
struct Section 
{
    unsigned char table_id;

    bool section_syntax_indicator;

    unsigned short section_length;

    unsigned char version_number;

    bool current_next_indicator;

    unsigned char section_number;

    unsigned char last_section_number;

    int CRC_32;

};
/**
 * CAT表.
 */
struct ConditionalAccessSection : public Section 
{
    class CA_info 
    {
    };
    
    unsigned char tableId;

};
/**
 * NIT表.
 */
struct NetworkInformationSection : public Section 
{
};
/**
 * PAT表项
 */
struct ProgramAssociationSection : public Section 
{
    unsigned short transport_stream_id;

    struct pat_info 
    {
        unsigned short program_number;

        unsigned short network_or_program_map_PID;

    };
    
    /**
     * network section 的PID.
     */
    vector<pat_info> NetworkPids;

    /**
     * program map 标识.
     */
    vector<pat_info> ProgramMapPids;

};
/**
 * PMT表项
 */
struct ProgramMapSection : public Section 
{
    struct Program_info 
    {
    };
    
    struct ES_info 
    {
        unsigned char stream_type;

        unsigned short elementary_PID;

        unsigned short ES_info_length;

    };
    
    /**
     * PCR的标识.
     */
    unsigned short PCR_PID;

    unsigned short program_info_length;

    Program_info ProgramInfo;

    vector<ES_info> esInfo;

    unsigned short program_number;

};
/**
 * SDT表
 */
struct TransportStreamDescriptionSection : public Section 
{
};
/**
 * TS包结构.
 */
struct TransportPacket 
{
    struct adaptation_field 
    {
        unsigned char adaptation_field_length;

        bool discontinuity_indicator;

        bool random_access_indicator;

        bool elementary_stream_priority_indicator;

        bool PCR_flag;

        bool OPCR_flag;

        bool splicing_point_flag;

        bool transport_private_data_flag;

        bool adaptation_field_extension_flag;

        /**
         * 33bit
         */
        double program_clock_reference_base;

        /**
         * 9
         */
        unsigned short program_clock_reference_extension;

        /**
         * 33bit
         */
        double original_program_clock_reference_base;

        /**
         * 9
         */
        unsigned short original_program_clock_reference_extension;

        unsigned char splice_countdown;

        unsigned char transport_private_data_length;

        unsigned char adaptation_field_extension_length;

        bool ltw_flag;

        bool piecewise_rate_flag;

        bool seamless_splice_flag;

        bool ltw_valid_flag;

        unsigned short ltw_offset;

        int piecewise_rate;

        unsigned char splice_type;

        double DTS_next_AU;

    };
    
    bool transportErrorIndicator;

    unsigned char syncByte;

    bool payloadUnitStartIndicator;

    bool transportPriority;

    int pid;

    unsigned char transportScramblingControl;

    unsigned char adaptationFieldControl;

    unsigned char continuityCounter;

    adaptation_field AdaptationField;

};
/**
 * pes包数据
 */
struct PESPacket 
{
    /**
     * 起始码前缀.
     */
    unsigned char packet_start_code_prefix[3];

    /**
     * 流ID.
     */
    unsigned char stream_id;

    unsigned short PES_packet_length;

    unsigned char PES_scrambling_control;

    bool PES_priority;

    bool data_alignment_indicator;

    bool copyright;

    bool original_or_copy;

    unsigned char PTS_DTS_flags;

    bool ESCR_flag;

    bool ES_rate_flag;

    bool DSM_trick_mode_flag;

    bool additional_copy_info_flag;

    bool PES_CRC_flag;

    bool PES_extension_flag;

    unsigned char PES_header_data_length;
#ifdef WIN32
	__int64 PTS;
	__int64 DTS;
#else
	long long PTS;
	long long DTS;
#endif

    double ESCR_base;

    unsigned short ESCR_extension;

    int ES_rate;

    unsigned char trick_mode_control;

    unsigned char field_id;

    bool intra_slice_refresh;

    unsigned char frequency_truncation;

    unsigned char rep_cntrl;

    bool marker_bit;

    unsigned char additional_copy_info;

    unsigned short previous_PES_packet_CRC;

    bool PES_private_data_flag;

    bool pack_header_field_flag;

    bool program_packet_sequence_counter_flag;

    bool P_STD_buffer_flag;

    bool PES_extension_flag_2;

    unsigned char PES_private_data[16];

    unsigned char pack_field_length;

    unsigned char program_packet_sequence_counter;

    bool MPEG1_MPEG2_identifier;

    unsigned char original_stuff_length;

    bool P_STD_buffer_scale;

    unsigned short P_STD_buffer_size;

    unsigned char PES_extension_field_length;

};
/**
 * EPG 电子节目单.
 */
struct EPG 
{
};
/**
 * 描述符
 */
struct Descriptor 
{
    /**
     * 描述符标识.
     */
    unsigned char descriptor_tag;

    /**
     * 描述符长度
     */
    unsigned char descriptor_length;

};
struct VideoStreamDescriptor : public Descriptor 
{
    bool multiple_frame_rate_flag;

    unsigned char frame_rate_code;

    bool MPEG_1_only_flag;

    bool constrained_parameter_flag;

    bool still_picture_flag;

    unsigned char profile_and_level_indication;

    bool frame_rate_extension_flag;

    unsigned char chroma_format;

};
struct AudioStreamDescriptor : public Descriptor 
{
    bool free_format_flag;

    bool ID;

    unsigned char layer;

    bool variable_rate_audio_indicator;

};
struct HierarchyDescriptor : public Descriptor 
{
    unsigned char reserved;

};
struct RegistrationDescriptor : public Descriptor 
{
    int format_identifier;

};
struct DataStreamAlignmentDescriptor : public Descriptor 
{
    unsigned char alignment_type;

};
struct TargetBackgroundGridDescriptor : public Descriptor 
{
    unsigned short horizontal_size;

};
struct VideoWindowDescriptor : public Descriptor 
{
    unsigned short horizontal_offset;

};
struct CADescriptor : public Descriptor 
{
    unsigned short CA_system_ID;

};
struct ISO639LanguageDescriptor : public Descriptor 
{
    struct LanguageSection 
    {
        int ISO_639_language_code;

        unsigned char audio_type;

    };
    
    vector<LanguageSection> languageSections;

};
struct SystemClockDescriptor : public Descriptor 
{
    bool external_clock_reference_indicator;

};
struct MultiplexBufferUtilizationDescriptor : public Descriptor 
{
    bool bound_valid_flag;

};
struct CopyrightDescriptor : public Descriptor 
{
    /**
     * 版权标识.
     */
    int copyright_identifier;

    vector<unsigned char> additional_copyright_info;

};
struct MaximumBitrateDescriptor : public Descriptor 
{
};
struct PrivateDataIndicatorDescriptor : public Descriptor 
{
    int private_data_indicator;

};
struct SmoothingBufferDescriptor : public Descriptor 
{
};
struct STDDescriptor : public Descriptor 
{
};
struct External_ES_IDDescriptor : public Descriptor 
{
};
struct FMCDescriptor : public Descriptor 
{
    struct FMCSection 
    {
    };
    
};
/**
 * FlexMuxBufferDescriptor(). It is defined in subclause 11.2 of ISO/IEC 14496-1.
 */
struct FmxBufferSizeDescriptor : public Descriptor 
{
};
struct IBPDescriptor : public Descriptor 
{
};
struct IODDescriptor : public Descriptor 
{
};
struct MPEG4AudioDescriptor : public Descriptor 
{
};
struct MPEG4VideoDescriptor : public Descriptor 
{
};
struct MultiplexBufferDescriptor : public Descriptor 
{
};
struct MuxcodeDescriptor : public Descriptor 
{
};
struct SLDescriptor : public Descriptor 
{
};
/**
 * Program Stream directory packet
 */
struct DirectoryPESPacket 
{
    struct AccessUnits 
    {
        unsigned char packet_stream_id;

        bool PES_header_position_offset_sign;

        double PES_header_position_offset;

        unsigned short reference_offset;

        double PTS;

        unsigned char bytes_to_read[23];

        bool intra_coded_indicator;

        unsigned char coding_parameters_indicator;

    };
    
    int packet_start_code_prefix;

    unsigned char directory_stream_id;

    unsigned short PES_packet_length;

    unsigned short number_of_access_units;

    double prev_directory_offset;

};
struct PackHeader 
{
    class SystemHeader 
    {
    public:
        int system_header_start_code;

        unsigned short header_length;

        int rate_bound;

        unsigned char audio_bound;

        unsigned char video_bound;

        bool fixed_flag;

        bool CSPS_flag;

        bool system_audio_lock_flag;

        bool system_video_lock_flag;

        bool packet_rate_restriction_flag;

    };
    
    int pack_start_code;

    double system_clock_reference_base;

    int program_mux_rate;

    unsigned char pack_stuffing_length;

    SystemHeader sysHeader;

};
/**
 * The PSM is present as a PES packet when the stream_id value is 0xBC
 */
struct ProgramStreamMap 
{
};

} // namespace wzd
#endif
