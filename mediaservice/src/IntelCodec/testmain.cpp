#if 0
#include "mfx_samples_config.h"
#include <iostream>
#include <memory>
#include "pipeline_encode.h"
#include "pipeline_user.h"
#include "pipeline_region_encode.h"
#include <stdarg.h>
#include <string>

#define VAL_CHECK(val, argIdx, argName) \
{ \
    if (val) \
{ \
    PrintHelp(NULL, MSDK_STRING("Input argument number %d \"%s\" require more parameters"), argIdx, argName); \
    return MFX_ERR_UNSUPPORTED;\
    } \
    }

// Extensions for internal use, normally these macros are blank
#ifdef MOD_ENC
#include "extension_macros.h"
#else
#define MOD_ENC_CREATE_PIPELINE
#define MOD_ENC_PRINT_HELP
#define MOD_ENC_PARSE_INPUT
#endif

void PrintHelp(msdk_char *strAppName, const msdk_char *strErrorMessage, ...)
{
    msdk_printf(MSDK_STRING("Encoding Sample Version %s\n\n"), MSDK_SAMPLE_VERSION);

    if (strErrorMessage)
    {
        va_list args;
        msdk_printf(MSDK_STRING("ERROR: "));
        va_start(args, strErrorMessage);
        msdk_vprintf(strErrorMessage, args);
        va_end(args);
        msdk_printf(MSDK_STRING("\n\n"));
    }

    msdk_printf(MSDK_STRING("Usage: %s <msdk-codecid> [<options>] -i InputYUVFile -o OutputEncodedFile -w width -h height\n"), strAppName);
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Supported codecs, <msdk-codecid>:\n"));
    msdk_printf(MSDK_STRING("   <codecid>=h264|mpeg2|vc1|mvc|jpeg - built-in Media SDK codecs\n"));
    msdk_printf(MSDK_STRING("   <codecid>=h265|vp8                - in-box Media SDK plugins (may require separate downloading and installation)\n"));
    msdk_printf(MSDK_STRING("   If codecid is jpeg, -q option is mandatory.)\n"));
    msdk_printf(MSDK_STRING("Options: \n"));
    MOD_ENC_PRINT_HELP;
    msdk_printf(MSDK_STRING("   [-nv12] - input is in NV12 color format, if not specified YUV420 is expected\n"));
    msdk_printf(MSDK_STRING("   [-tff|bff] - input stream is interlaced, top|bottom fielf first, if not specified progressive is expected\n"));
    msdk_printf(MSDK_STRING("   [-bref] - arrange B frames in B pyramid reference structure\n"));
    msdk_printf(MSDK_STRING("   [-nobref] -  do not use B-pyramid (by default the decision is made by library)\n"));
    msdk_printf(MSDK_STRING("   [-idr_interval size] - idr interval, default 0 means every I is an IDR, 1 means every other I frame is an IDR etc\n"));
    msdk_printf(MSDK_STRING("   [-f frameRate] - video frame rate (frames per second)\n"));
    msdk_printf(MSDK_STRING("   [-n number] - number of frames to process\n"));
    msdk_printf(MSDK_STRING("   [-b bitRate] - encoded bit rate (Kbits per second), valid for H.264, H.265, MPEG2 and MVC encoders \n"));
    msdk_printf(MSDK_STRING("   [-u speed|quality|balanced] - target usage, valid for H.264, H.265, MPEG2 and MVC encoders\n"));
    msdk_printf(MSDK_STRING("   [-q quality] - mandatory quality parameter for JPEG encoder. In range [1,100]. 100 is the best quality. \n"));
    msdk_printf(MSDK_STRING("   [-r distance] - Distance between I- or P- key frames (1 means no B-frames) \n"));
    msdk_printf(MSDK_STRING("   [-g size] - GOP size (default 256)\n"));
    msdk_printf(MSDK_STRING("   [-x numRefs]   - number of reference frames\n"));
    msdk_printf(MSDK_STRING("   [-la] - use the look ahead bitrate control algorithm (LA BRC) (by default constant bitrate control method is used)\n"));
    msdk_printf(MSDK_STRING("           for H.264, H.265 encoder. Supported only with -hw option on 4th Generation Intel Core processors. \n"));
    msdk_printf(MSDK_STRING("   [-lad depth] - depth parameter for the LA BRC, the number of frames to be analyzed before encoding. In range [10,100].\n"));
    msdk_printf(MSDK_STRING("            may be 1 in the case when -mss option is specified \n"));
    msdk_printf(MSDK_STRING("   [-dstw width] - destination picture width, invokes VPP resizing\n"));
    msdk_printf(MSDK_STRING("   [-dsth height] - destination picture height, invokes VPP resizing\n"));
    msdk_printf(MSDK_STRING("   [-hw] - use platform specific SDK implementation (default)\n"));
    msdk_printf(MSDK_STRING("   [-sw] - use software implementation, if not specified platform specific SDK implementation is used\n"));
    msdk_printf(MSDK_STRING("   [-p guid] - 32-character hexadecimal guid string\n"));
    msdk_printf(MSDK_STRING("                              (optional for Media SDK in-box plugins, required for user-encoder ones)\n"));
    msdk_printf(MSDK_STRING("   [-path path] - path to plugin (valid only in pair with -p option)\n"));
    msdk_printf(MSDK_STRING("   [-async]                 - depth of asynchronous pipeline. default value is 4. must be between 1 and 20.\n"));
    msdk_printf(MSDK_STRING("   [-gpucopy::<on,off>] Enable or disable GPU copy mode\n"));
    msdk_printf(MSDK_STRING("   [-cqp]                   - constant quantization parameter (CQP BRC) bitrate control method\n"));
    msdk_printf(MSDK_STRING("                              (by default constant bitrate control method is used), should be used along with -qpi, -qpp, -qpb.\n"));
    msdk_printf(MSDK_STRING("   [-qpi]                   - constant quantizer for I frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING("   [-qpp]                   - constant quantizer for P frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING("   [-qpb]                   - constant quantizer for B frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING("   [-num_slice]             - number of slices in each video frame. 0 by default.\n"));
    msdk_printf(MSDK_STRING("                              If num_slice equals zero, the encoder may choose any slice partitioning allowed by the codec standard.\n"));
    msdk_printf(MSDK_STRING("   [-mss]                   - maximum slice size in bytes. Supported only with -hw and h264 codec. This option is not compatible with -num_slice option.\n"));
    msdk_printf(MSDK_STRING("   [-re]                    - enable region encode mode. Works only with h265 encoder\n"));
    msdk_printf(MSDK_STRING("Example: %s h265 -i InputYUVFile -o OutputEncodedFile -w width -h height -hw -p 2fca99749fdb49aeb121a5b63ef568f7\n"), strAppName);
#if D3D_SURFACES_SUPPORT
    msdk_printf(MSDK_STRING("   [-d3d] - work with d3d surfaces\n"));
    msdk_printf(MSDK_STRING("   [-d3d11] - work with d3d11 surfaces\n"));
    msdk_printf(MSDK_STRING("Example: %s h264|h265|mpeg2|jpeg -i InputYUVFile -o OutputEncodedFile -w width -h height -d3d -hw \n"), strAppName);
    msdk_printf(MSDK_STRING("Example for MVC: %s mvc -i InputYUVFile_1 -i InputYUVFile_2 -o OutputEncodedFile -w width -h height \n"), strAppName);
#endif
#ifdef LIBVA_SUPPORT
    msdk_printf(MSDK_STRING("   [-vaapi] - work with vaapi surfaces\n"));
    msdk_printf(MSDK_STRING("Example: %s h264|mpeg2|mvc -i InputYUVFile -o OutputEncodedFile -w width -h height -angle 180 -g 300 -r 1 \n"), strAppName);
#endif
#if defined (ENABLE_V4L2_SUPPORT)
    msdk_printf(MSDK_STRING("   [-d]                            - Device video node (eg: /dev/video0)\n"));
    msdk_printf(MSDK_STRING("   [-p]                            - Mipi Port number (eg: Port 0)\n"));
    msdk_printf(MSDK_STRING("   [-m]                            - Mipi Mode Configuration [PREVIEW/CONTINUOUS/STILL/VIDEO]\n"));
    msdk_printf(MSDK_STRING("   [-uyvy]                        - Input Raw format types V4L2 Encode\n"));
    msdk_printf(MSDK_STRING("   [-YUY2]                        - Input Raw format types V4L2 Encode\n"));
    msdk_printf(MSDK_STRING("   [-i::v4l2]                        - To enable v4l2 option\n"));
    msdk_printf(MSDK_STRING("Example: %s h264|mpeg2|mvc -i::v4l2 -o OutputEncodedFile -w width -h height -d /dev/video0 -uyvy -m preview -p 0\n"), strAppName);
#endif
    msdk_printf(MSDK_STRING("   [-viewoutput] - instruct the MVC encoder to output each view in separate bitstream buffer. Depending on the number of -o options behaves as follows:\n"));
    msdk_printf(MSDK_STRING("                   1: two views are encoded in single file\n"));
    msdk_printf(MSDK_STRING("                   2: two views are encoded in separate files\n"));
    msdk_printf(MSDK_STRING("                   3: behaves like 2 -o opitons was used and then one -o\n\n"));
    msdk_printf(MSDK_STRING("Example: %s mvc -i InputYUVFile_1 -i InputYUVFile_2 -o OutputEncodedFile_1 -o OutputEncodedFile_2 -viewoutput -w width -h height \n"), strAppName);
    // user module options
    msdk_printf(MSDK_STRING("User module options: \n"));
    msdk_printf(MSDK_STRING("   [-angle 180] - enables 180 degrees picture rotation before encoding, CPU implementation by default. Rotation requires NV12 input. Options -tff|bff, -dstw, -dsth, -d3d are not effective together with this one, -nv12 is required.\n"));
    msdk_printf(MSDK_STRING("   [-opencl] - rotation implementation through OPENCL\n"));
    msdk_printf(MSDK_STRING("Example: %s h264|h265|mpeg2|mvc|jpeg -i InputYUVFile -o OutputEncodedFile -w width -h height -angle 180 -opencl \n"), strAppName);

    msdk_printf(MSDK_STRING("\n"));
}

mfxStatus ParseInputString(msdk_char* strInput[], mfxU8 nArgNum, sInputParams* pParams)
{

    if (1 == nArgNum)
    {
        PrintHelp(strInput[0], NULL);
        return MFX_ERR_UNSUPPORTED;
    }

    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    msdk_opt_read(MSDK_CPU_ROTATE_PLUGIN, pParams->strPluginDLLPath);

    // default implementation
    pParams->bUseHWLib = true;
    pParams->isV4L2InputEnabled = false;
    pParams->nNumFrames = 0;
#if defined (ENABLE_V4L2_SUPPORT)
    pParams->MipiPort = -1;
    pParams->MipiMode = NONE;
    pParams->v4l2Format = NO_FORMAT;
#endif

    // parse command line parameters
    for (mfxU8 i = 1; i < nArgNum; i++)
    {
        MSDK_CHECK_POINTER(strInput[i], MFX_ERR_NULL_PTR);

        if (MSDK_CHAR('-') != strInput[i][0])
        {
            mfxStatus sts = StrFormatToCodecFormatFourCC(strInput[i], pParams->CodecId);
            if (sts != MFX_ERR_NONE)
            {
                PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (!IsEncodeCodecSupported(pParams->CodecId))
            {
                PrintHelp(strInput[0], MSDK_STRING("Unsupported codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (pParams->CodecId == CODEC_MVC)
            {
                pParams->CodecId = MFX_CODEC_AVC;
                pParams->MVC_flags |= MVC_ENABLED;
            }
            continue;
        }

        // process multi-character options
        if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dstw")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nDstWidth))
            {
                PrintHelp(strInput[0], MSDK_STRING("Destination picture Width is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dsth")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nDstHeight))
            {
                PrintHelp(strInput[0], MSDK_STRING("Destination picture Height is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-sw")))
        {
            pParams->bUseHWLib = false;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-hw")))
        {
            pParams->bUseHWLib = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-nv12")))
        {
            pParams->ColorFormat = MFX_FOURCC_NV12;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-tff")))
        {
            pParams->nPicStruct = MFX_PICSTRUCT_FIELD_TFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-bff")))
        {
            pParams->nPicStruct = MFX_PICSTRUCT_FIELD_BFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-bref")))
        {
            pParams->nBRefType = MFX_B_REF_PYRAMID;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-nobref")))
        {
            pParams->nBRefType = MFX_B_REF_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-idr_interval")))
        {
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nIdrInterval))
            {
                PrintHelp(strInput[0], MSDK_STRING("IdrInterval is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-angle")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nRotationAngle))
            {
                PrintHelp(strInput[0], MSDK_STRING("Rotation Angle is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-opencl")))
        {
            msdk_opt_read(MSDK_OCL_ROTATE_PLUGIN, pParams->strPluginDLLPath);
            pParams->nRotationAngle = 180;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-viewoutput")))
        {
            if (!(MVC_ENABLED & pParams->MVC_flags))
            {
                PrintHelp(strInput[0], MSDK_STRING("-viewoutput option is supported only when mvc codec specified"));
                return MFX_ERR_UNSUPPORTED;
            }
            pParams->MVC_flags |= MVC_VIEWOUTPUT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-la")))
        {
            pParams->nRateControlMethod = MFX_RATECONTROL_LA;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-lad")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            pParams->nRateControlMethod = MFX_RATECONTROL_LA;
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nLADepth))
            {
                PrintHelp(strInput[0], MSDK_STRING("Look Ahead Depth is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-mss")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nMaxSliceSize))
            {
                PrintHelp(strInput[0], MSDK_STRING("MaxSliceSize is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
#if D3D_SURFACES_SUPPORT
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d")))
        {
            pParams->memType = D3D9_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d11")))
        {
            pParams->memType = D3D11_MEMORY;
        }
#endif
#ifdef LIBVA_SUPPORT
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-vaapi")))
        {
            pParams->memType = D3D9_MEMORY;
        }
#endif
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-async")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nAsyncDepth))
            {
                PrintHelp(strInput[0], MSDK_STRING("Async Depth is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::on")))
        {
            pParams->gpuCopy = MFX_GPUCOPY_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::off")))
        {
            pParams->gpuCopy = MFX_GPUCOPY_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-cqp")))
        {
            pParams->nRateControlMethod = MFX_RATECONTROL_CQP;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpi")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQPI))
            {
                PrintHelp(strInput[0], MSDK_STRING("Quantizer for I frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpp")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQPP))
            {
                PrintHelp(strInput[0], MSDK_STRING("Quantizer for P frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpb")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQPB))
            {
                PrintHelp(strInput[0], MSDK_STRING("Quantizer for B frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-num_slice")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nNumSlice))
            {
                PrintHelp(strInput[0], MSDK_STRING("Number of slices is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        } else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-path")))
        {
            i++;
#if defined(_WIN32) || defined(_WIN64)
            msdk_char wchar[MSDK_MAX_FILENAME_LEN];
            msdk_opt_read(strInput[i], wchar);
            std::wstring wstr(wchar);
            std::string str(wstr.begin(), wstr.end());

            strcpy_s(pParams->pluginParams.strPluginPath, str.c_str());
#else
            msdk_opt_read(strInput[i], pParams->pluginParams.strPluginPath);
#endif
            pParams->pluginParams.type = MFX_PLUGINLOAD_TYPE_FILE;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-re")))
        {
            pParams->UseRegionEncode = true;
        }
        MOD_ENC_PARSE_INPUT
        #if defined (ENABLE_V4L2_SUPPORT)
                else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->DeviceName))
            {
                PrintHelp(strInput[0], MSDK_STRING("Device name is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-uyvy")))
        {
            pParams->v4l2Format = UYVY;

        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-yuy2")))
        {
            pParams->v4l2Format = YUY2;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->MipiPort))
            {
                PrintHelp(strInput[0], MSDK_STRING("Mipi-port is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-m")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->MipiModeName))
            {
                PrintHelp(strInput[0], MSDK_STRING("Device name is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }

            if(strcasecmp(pParams->MipiModeName,"STILL") == 0)
                pParams->MipiMode = STILL;
            else if(strcasecmp(pParams->MipiModeName,"VIDEO") == 0)
                pParams->MipiMode = VIDEO;
            else if(strcasecmp(pParams->MipiModeName,"PREVIEW") == 0)
                pParams->MipiMode = PREVIEW;
            else if(strcasecmp(pParams->MipiModeName,"CONTINUOUS") == 0)
                pParams->MipiMode = CONTINUOUS;
            else
                pParams->MipiMode = NONE;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i::v4l2")))
        {
            pParams->isV4L2InputEnabled = true;
        }
#endif
        else // 1-character options
        {
            switch (strInput[i][1])
            {
            case MSDK_CHAR('u'):
                if (++i < nArgNum) {
                    pParams->nTargetUsage = StrToTargetUsage(strInput[i]);
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-u' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('w'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nWidth))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Width is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-w' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('h'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nHeight))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Height is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-h' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('f'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->dFrameRate))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Frame Rate is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-f' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('n'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nNumFrames))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Number of frames to process is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-n' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('b'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nBitRate))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Bit Rate is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-b' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('x'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nNumRefFrame))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Ref Num is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-x' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('g'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nGopPicSize))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Gop Size is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-g' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('r'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nGopRefDist))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Ref Dist is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-r' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('i'):
                if (++i < nArgNum) {
                    msdk_opt_read(strInput[i], pParams->strSrcFile);
                    if (MVC_ENABLED & pParams->MVC_flags)
                    {
                        pParams->srcFileBuff.push_back(strInput[i]);
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-i' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('o'):
                if (++i < nArgNum) {
                    pParams->dstFileBuff.push_back(strInput[i]);
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-o' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('q'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nQuality))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Quality is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-q' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('p'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE == ConvertStringToGuid(strInput[i], pParams->pluginParams.pluginGuid))
                    {
                        pParams->pluginParams.type = MFX_PLUGINLOAD_TYPE_GUID;
                    }
                    else
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Unknown options"));
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-p' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('?'):
                PrintHelp(strInput[0], NULL);
                return MFX_ERR_UNSUPPORTED;
            default:
                PrintHelp(strInput[0], MSDK_STRING("Unknown options"));
            }
        }
    }

#if defined (ENABLE_V4L2_SUPPORT)
    if (pParams->isV4L2InputEnabled)
    {
        if (0 == msdk_strlen(pParams->DeviceName))
        {
            PrintHelp(strInput[0], MSDK_STRING("Device Name not found"));
            return MFX_ERR_UNSUPPORTED;
        }

        if ((pParams->MipiPort > -1 && pParams->MipiMode == NONE) ||
                (pParams->MipiPort < 0 && pParams->MipiMode != NONE))
        {
            PrintHelp(strInput[0], MSDK_STRING("Invalid Mipi Configuration\n"));
            return MFX_ERR_UNSUPPORTED;
        }

        if (pParams->v4l2Format == NO_FORMAT)
        {
            PrintHelp(strInput[0], MSDK_STRING("NO input v4l2 format\n"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
#endif

    // check if all mandatory parameters were set
    if (0 == msdk_strlen(pParams->strSrcFile) && !pParams->isV4L2InputEnabled)
    {
        PrintHelp(strInput[0], MSDK_STRING("Source file name not found"));
        return MFX_ERR_UNSUPPORTED;
    };

    if (pParams->dstFileBuff.empty())
    {
        PrintHelp(strInput[0], MSDK_STRING("Destination file name not found"));
        return MFX_ERR_UNSUPPORTED;
    };

    if (0 == pParams->nWidth || 0 == pParams->nHeight)
    {
        PrintHelp(strInput[0], MSDK_STRING("-w, -h must be specified"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_MPEG2 != pParams->CodecId &&
            MFX_CODEC_AVC != pParams->CodecId &&
            MFX_CODEC_JPEG != pParams->CodecId &&
            MFX_CODEC_VP8 != pParams->CodecId &&
            MFX_CODEC_HEVC != pParams->CodecId)
    {
        PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
        return MFX_ERR_UNSUPPORTED;
    }

    // check parameters validity
    if (pParams->nRotationAngle != 0 && pParams->nRotationAngle != 180)
    {
        PrintHelp(strInput[0], MSDK_STRING("Angles other than 180 degrees are not supported."));
        return MFX_ERR_UNSUPPORTED; // other than 180 are not supported
    }

    if (pParams->nQuality && (MFX_CODEC_JPEG != pParams->CodecId))
    {
        PrintHelp(strInput[0], MSDK_STRING("-q option is supported only for JPEG encoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nTargetUsage || pParams->nBitRate) && (MFX_CODEC_JPEG == pParams->CodecId))
    {
        PrintHelp(strInput[0], MSDK_STRING("-u and -b options are supported only for H.264, MPEG2 and MVC encoders. For JPEG encoder use -q"));
        return MFX_ERR_UNSUPPORTED;
    }

    // set default values for optional parameters that were not set or were set incorrectly
    mfxU32 nviews = (mfxU32)pParams->srcFileBuff.size();
    if ((nviews <= 1) || (nviews > 2))
    {
        if (!(MVC_ENABLED & pParams->MVC_flags))
        {
            pParams->numViews = 1;
        }
        else
        {
            PrintHelp(strInput[0], MSDK_STRING("Only 2 views are supported right now in this sample."));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else
    {
        pParams->numViews = nviews;
    }

    if (MFX_TARGETUSAGE_BEST_QUALITY != pParams->nTargetUsage && MFX_TARGETUSAGE_BEST_SPEED != pParams->nTargetUsage)
    {
        pParams->nTargetUsage = MFX_TARGETUSAGE_BALANCED;
    }

    if (pParams->dFrameRate <= 0)
    {
        pParams->dFrameRate = 30;
    }

    // if no destination picture width or height wasn't specified set it to the source picture size
    if (pParams->nDstWidth == 0)
    {
        pParams->nDstWidth = pParams->nWidth;
    }

    if (pParams->nDstHeight == 0)
    {
        pParams->nDstHeight = pParams->nHeight;
    }

    // calculate default bitrate based on the resolution (a parameter for encoder, so Dst resolution is used)
    if (pParams->nBitRate == 0)
    {
        pParams->nBitRate = CalculateDefaultBitrate(pParams->CodecId, pParams->nTargetUsage, pParams->nDstWidth,
                                                    pParams->nDstHeight, pParams->dFrameRate);
    }

    // if nv12 option wasn't specified we expect input YUV file in YUV420 color format
    if (!pParams->ColorFormat)
    {
        pParams->ColorFormat = MFX_FOURCC_YV12;
    }

    if (!pParams->nPicStruct)
    {
        pParams->nPicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    }

    if ((pParams->nRateControlMethod == MFX_RATECONTROL_LA) && (!pParams->bUseHWLib))
    {
        PrintHelp(strInput[0], MSDK_STRING("Look ahead BRC is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nMaxSliceSize) && (!pParams->bUseHWLib))
    {
        PrintHelp(strInput[0], MSDK_STRING("MaxSliceSize option is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nMaxSliceSize) && (pParams->nNumSlice))
    {
        PrintHelp(strInput[0], MSDK_STRING("-mss and -num_slice options are not compatible!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nRateControlMethod == MFX_RATECONTROL_LA) && (pParams->CodecId != MFX_CODEC_AVC))
    {
        PrintHelp(strInput[0], MSDK_STRING("Look ahead BRC is supported only with H.264 encoder!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nMaxSliceSize) && (pParams->CodecId != MFX_CODEC_AVC))
    {
        PrintHelp(strInput[0], MSDK_STRING("MaxSliceSize option is supported only with H.264 encoder!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (pParams->nLADepth && (pParams->nLADepth < 10 || pParams->nLADepth > 100))
    {
        if ((pParams->nLADepth != 1) || (!pParams->nMaxSliceSize))
        {
            PrintHelp(strInput[0], MSDK_STRING("Unsupported value of -lad parameter, must be in range [10, 100] or 1 in case of -mss option!"));
            return MFX_ERR_UNSUPPORTED;
        }
    }

    // not all options are supported if rotate plugin is enabled
    if (pParams->nRotationAngle == 180 && (
                MFX_PICSTRUCT_PROGRESSIVE != pParams->nPicStruct ||
                pParams->nDstWidth != pParams->nWidth ||
                pParams->nDstHeight != pParams->nHeight ||
                MVC_ENABLED & pParams->MVC_flags ||
                pParams->nRateControlMethod == MFX_RATECONTROL_LA))
    {
        PrintHelp(strInput[0], MSDK_STRING("Some of the command line options are not supported with rotation plugin!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (pParams->nAsyncDepth == 0)
    {
        pParams->nAsyncDepth = 4; //set by default;
    }

    // Ignoring user-defined Async Depth for LA
    if (pParams->nMaxSliceSize)
    {
        pParams->nAsyncDepth = 1;
    }

    if (pParams->nRateControlMethod == 0)
    {
        pParams->nRateControlMethod = MFX_RATECONTROL_CBR;
    }

    if(pParams->UseRegionEncode)
    {
        if(pParams->CodecId != MFX_CODEC_HEVC)
        {
            msdk_printf(MSDK_STRING("Region encode option is compatible with h265(HEVC) encoder only.\nRegion encoding is disabled\n"));
            pParams->UseRegionEncode=false;
        }
        if (pParams->nWidth  != pParams->nDstWidth ||
                pParams->nHeight != pParams->nDstHeight ||
                pParams->nRotationAngle!=0)

        {
            msdk_printf(MSDK_STRING("Region encode option is not compatible with VPP processing.\nRegion encoding is disabled\n"));
            pParams->UseRegionEncode=false;
        }
    }

    return MFX_ERR_NONE;
}

CEncodingPipeline* CreatePipeline(const sInputParams& params)
{
    MOD_ENC_CREATE_PIPELINE;

    if(params.UseRegionEncode)
    {
        //return new CRegionEncodingPipeline;
    }
    else if(params.nRotationAngle)
    {
        //return new CUserPipeline;
    }
    else
    {
        return new CEncodingPipeline;
    }
}

using namespace std;
#if defined(_WIN32) || defined(_WIN64)
int _tmain(int argc, msdk_char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    sInputParams Params = {};   // input parameters from command line
    std::auto_ptr<CEncodingPipeline>  pPipeline;

    mfxStatus sts = MFX_ERR_NONE; // return value check

    sts = ParseInputString(argv, (mfxU8)argc, &Params);
    MSDK_CHECK_PARSE_RESULT(sts, MFX_ERR_NONE, 1);

    // Choosing which pipeline to use
    pPipeline.reset(CreatePipeline(Params));

    MSDK_CHECK_POINTER(pPipeline.get(), MFX_ERR_MEMORY_ALLOC);

    if (MVC_ENABLED & Params.MVC_flags)
    {
        pPipeline->SetMultiView();
        pPipeline->SetNumView(Params.numViews);
    }
    //Params.nGopPicSize = 10;
    //Params.nBRefType = MFX_B_REF_OFF;
    {
        std::cout<<"nTargetUsage\t" << Params.nTargetUsage<< endl;
        std::cout<<"CodecId\t" << Params.CodecId<< endl;
        std::cout<<"ColorFormat\t" << Params.ColorFormat<< endl; // YV12
        std::cout<<"nPicStruct\t" << Params.nPicStruct<< endl;
        std::cout<<"nWidth\t" << Params.nWidth<< endl;
        std::cout<<"nHeight\t" << Params.nHeight<< endl;
        std::cout<<"dFrameRate\t" << Params.dFrameRate<< endl;
        std::cout<<"nNumFrames\t" << Params.nNumFrames<< endl;
        std::cout<<"nBitRate\t" << Params.nBitRate<< endl;
        std::cout<<"MVC_flags\t" << Params.MVC_flags<< endl;
        std::cout<<"nGopPicSize\t" << Params.nGopPicSize<< endl;
        std::cout<<"nGopRefDist\t" << Params.nGopRefDist<< endl;
        std::cout<<"nNumRefFrame\t" << Params.nNumRefFrame<< endl;
        std::cout<<"nBRefType\t" << Params.nBRefType<< endl;
        std::cout<<"nIdrInterval\t" << Params.nIdrInterval<< endl;

        std::cout<<"nQuality\t" << Params.nQuality<< endl;

        std::cout<<"numViews\t" << Params.numViews<< endl;

        std::cout<<"nDstWidth\t" << Params.nDstWidth<< endl;
        std::cout<<"nDstHeight\t" << Params.nDstHeight<< endl;

        std::cout<<"memType\t" << Params.memType<< endl;
        std::cout<<"bUseHWLib\t" << Params.bUseHWLib<< endl;

        cout<<"strSrcFile\t" << (char*)(Params.strSrcFile)<< endl;

        std::cout<<"pluginParams.pluginGuid\t" << Params.pluginParams.pluginGuid.Data<< endl;
        std::cout<<"pluginParams.strPluginPath\t" << Params.pluginParams.strPluginPath<< endl;
        std::cout<<"pluginParams.type\t" << Params.pluginParams.type<< endl;

        std::cout<<".srcFileBuff.size()\t" << Params.srcFileBuff.size()<< endl;
        std::cout<<"dstFileBuff.size()\t" << Params.dstFileBuff.size()<< endl;

        std::cout<<"HEVCPluginVersion\t" << Params.HEVCPluginVersion<< endl;
        std::cout<<"nRotationAngle\t" << Params.nRotationAngle<< endl; // if specified, enables rotation plugin in mfx pipeline
        std::cout<<"strPluginDLLPath\t" << Params.strPluginDLLPath<<endl; // plugin dll path and name

        std::cout<<"nAsyncDepth\t" << Params.nAsyncDepth<< endl; // depth of asynchronous pipeline, this number can be tuned to achieve better performance
        std::cout<<"gpuCopy\t" << Params.gpuCopy<< endl; // GPU Copy mode (three-state option)

        std::cout<<"nRateControlMethod\t" << Params.nRateControlMethod<< endl;
        std::cout<<"nLADepth\t" << Params.nLADepth<< endl; // depth of the look ahead bitrate control  algorithm
        std::cout<<"nMaxSliceSize\t" << Params.nMaxSliceSize<< endl; //maximum size of slice
        std::cout<<"nQPI\t" << Params.nQPI<< endl;
        std::cout<<"nQPP\t" << Params.nQPP<< endl;
        std::cout<<"nQPB\t" << Params.nQPB<< endl;

        std::cout<<"nNumSlice\t" << Params.nNumSlice<< endl;
        std::cout<<"UseRegionEncode\t" << Params.UseRegionEncode<< endl;

        std::cout<<"isV4L2InputEnabled\t" << Params.isV4L2InputEnabled;
    }


    sts = pPipeline->Init(&Params);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    pPipeline->PrintInfo();

    msdk_printf(MSDK_STRING("Processing started\n"));

    if (pPipeline->CaptureStartV4L2Pipeline() != MFX_ERR_NONE)
    {
        msdk_printf(MSDK_STRING("V4l2 failure terminating the program\n"));
        return 0;
    }
    outter otr;
    FILE* fp = fopen("writter.h265", "wb");
    int i = 100;
    while (--i)
    {
        sts = pPipeline->Run(otr);

        if (otr.used != 0)
        {
            fwrite(otr.pData, 1, otr.used, fp);
            std::cout << "isK " << otr.isKeyFrame<<'\t'<< "timeStamp: " << otr.timeStamp << "\tdecodetimeStamp: "<< otr.dts << endl;
        }
        if (MFX_ERR_NONE <= sts /*|| MFX_ERR_MORE_DATA == sts*/)
        {
            continue;
        }
        // means that the input file has ended, need to go to buffering loops
        MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_DATA);
        // exit in case of other errors
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        if (MFX_ERR_DEVICE_LOST == sts || MFX_ERR_DEVICE_FAILED == sts)
        {
            msdk_printf(MSDK_STRING("\nERROR: Hardware device was lost or returned an unexpected error. Recovering...\n"));
            sts = pPipeline->ResetDevice();
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

            sts = pPipeline->ResetMFXComponents(&Params);
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
            continue;
        }
        else
        {
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
            break;
        }
    }
    otr.used = 0;
        while (MFX_ERR_NONE <= pPipeline->Flush(otr))
        {
            fwrite(otr.pData, 1, otr.used, fp);
            otr.used = 0;
        }
        fwrite(otr.pData, 1, otr.used, fp);
        otr.used = 0;
        while(MFX_ERR_NONE == pPipeline->Flush2(otr))
        {
            fwrite(otr.pData, 1, otr.used, fp);
        }
    pPipeline->ResetMFXComponents(&Params);
        for (;;)
        {
            sts = pPipeline->Run(otr);

            if (otr.used != 0)
            {
                fwrite(otr.pData, 1, otr.used, fp);
                std::cout << "isK " << otr.isKeyFrame<<'\t'<< "ts: " << otr.timeStamp << endl;
            }
            if (MFX_ERR_NONE <= sts /*|| MFX_ERR_MORE_DATA == sts*/)
            {
                continue;
            }
            // means that the input file has ended, need to go to buffering loops
            MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_DATA);
            // exit in case of other errors
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
            if (MFX_ERR_DEVICE_LOST == sts || MFX_ERR_DEVICE_FAILED == sts)
            {
                msdk_printf(MSDK_STRING("\nERROR: Hardware device was lost or returned an unexpected error. Recovering...\n"));
                sts = pPipeline->ResetDevice();
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

                sts = pPipeline->ResetMFXComponents(&Params);
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
                continue;
            }
            else
            {
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
                break;
            }
        }
        otr.used = 0;
        while (MFX_ERR_NONE <= pPipeline->Flush(otr))
        {
            fwrite(otr.pData, 1, otr.used, fp);
            otr.used = 0;
        }
        fwrite(otr.pData, 1, otr.used, fp);
        otr.used = 0;
        while(MFX_ERR_NONE == pPipeline->Flush2(otr))
        {
            fwrite(otr.pData, 1, otr.used, fp);
        }

    fclose(fp);
    pPipeline->CaptureStopV4L2Pipeline();

    pPipeline->Close();

    msdk_printf(MSDK_STRING("\nProcessing finished\n"));

    return 0;
}

#else


#include "mfx_samples_config.h"

#include "pipeline_decode.h"
#include <sstream>

void PrintHelp(msdk_char *strAppName, const msdk_char *strErrorMessage)
{
    msdk_printf(MSDK_STRING("Decoding Sample Version %s\n\n"), MSDK_SAMPLE_VERSION);

    if (strErrorMessage)
    {
        msdk_printf(MSDK_STRING("Error: %s\n"), strErrorMessage);
    }

    msdk_printf(MSDK_STRING("Usage: %s <codecid> [<options>] -i InputBitstream\n"), strAppName);
    msdk_printf(MSDK_STRING("   or: %s <codecid> [<options>] -i InputBitstream -r\n"), strAppName);
    msdk_printf(MSDK_STRING("   or: %s <codecid> [<options>] -i InputBitstream -o OutputYUVFile\n"), strAppName);
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Supported codecs (<codecid>):\n"));
    msdk_printf(MSDK_STRING("   <codecid>=h264|mpeg2|vc1|mvc|jpeg - built-in Media SDK codecs\n"));
    msdk_printf(MSDK_STRING("   <codecid>=h265|capture            - in-box Media SDK plugins (may require separate downloading and installation)\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Work models:\n"));
    msdk_printf(MSDK_STRING("  1. Performance model: decoding on MAX speed, no rendering, no YUV dumping (no -r or -o option)\n"));
    msdk_printf(MSDK_STRING("  2. Rendering model: decoding with rendering on the screen (-r option)\n"));
    msdk_printf(MSDK_STRING("  3. Dump model: decoding with YUV dumping (-o option)\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Options:\n"));
    msdk_printf(MSDK_STRING("   [-hw]                     - use platform specific SDK implementation (default)\n"));
    msdk_printf(MSDK_STRING("   [-sw]                     - use software implementation, if not specified platform specific SDK implementation is used\n"));
    msdk_printf(MSDK_STRING("   [-p guid]                 - 32-character hexadecimal guid string\n"));
    msdk_printf(MSDK_STRING("   [-path path]              - path to plugin (valid only in pair with -p option)\n"));
    msdk_printf(MSDK_STRING("                               (optional for Media SDK in-box plugins, required for user-decoder ones)\n"));
    msdk_printf(MSDK_STRING("   [-f]                      - rendering framerate\n"));
    msdk_printf(MSDK_STRING("   [-w]                      - output width\n"));
    msdk_printf(MSDK_STRING("   [-h]                      - output height\n"));
    msdk_printf(MSDK_STRING("   [-di bob/adi]             - enable deinterlacing BOB/ADI\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Output format parameters:\n"));
    msdk_printf(MSDK_STRING("   [-i420] - pipeline output format: NV12, output file format: I420\n"));
    msdk_printf(MSDK_STRING("   [-nv12] - pipeline output format: NV12, output file format: NV12\n"));
    msdk_printf(MSDK_STRING("   [-rgb4] - pipeline output format: RGB4, output file format: RGB4\n"));
    msdk_printf(MSDK_STRING("   [-p010] - pipeline output format: P010, output file format: P010\n"));
    msdk_printf(MSDK_STRING("   [-a2rgb10] - pipeline output format: A2RGB10, output file format: A2RGB10\n"));
    msdk_printf(MSDK_STRING("\n"));
#if D3D_SURFACES_SUPPORT
    msdk_printf(MSDK_STRING("   [-d3d]                    - work with d3d9 surfaces\n"));
    msdk_printf(MSDK_STRING("   [-d3d11]                  - work with d3d11 surfaces\n"));
    msdk_printf(MSDK_STRING("   [-r]                      - render decoded data in a separate window \n"));
    msdk_printf(MSDK_STRING("   [-wall w h n m t tmo]     - same as -r, and positioned rendering window in a particular cell on specific monitor \n"));
    msdk_printf(MSDK_STRING("       w                     - number of columns of video windows on selected monitor\n"));
    msdk_printf(MSDK_STRING("       h                     - number of rows of video windows on selected monitor\n"));
    msdk_printf(MSDK_STRING("       n(0,.,w*h-1)          - order of video window in table that will be rendered\n"));
    msdk_printf(MSDK_STRING("       m(0,1..)              - monitor id \n"));
    msdk_printf(MSDK_STRING("       t(0/1)                - enable/disable window's title\n"));
    msdk_printf(MSDK_STRING("       tmo                   - timeout for -wall option\n"));
    msdk_printf(MSDK_STRING("Screen capture parameters:\n"));
    msdk_printf(MSDK_STRING("   [-scr:w]                  - screen resolution width\n"));
    msdk_printf(MSDK_STRING("   [-scr:h]                  - screen resolution height\n"));
    msdk_printf(MSDK_STRING("\n"));

#endif
#if defined(LIBVA_SUPPORT)
    msdk_printf(MSDK_STRING("   [-vaapi]                  - work with vaapi surfaces\n"));
#endif
#if defined(LIBVA_X11_SUPPORT)
    msdk_printf(MSDK_STRING("   [-r]                      - render decoded data in a separate X11 window \n"));
#endif
#if defined(LIBVA_WAYLAND_SUPPORT)
    msdk_printf(MSDK_STRING("   [-rwld]                   - render decoded data in a Wayland window \n"));
    msdk_printf(MSDK_STRING("   [-perf]                   - turn on asynchronous flipping for Wayland rendering \n"));
#endif
#if defined(LIBVA_DRM_SUPPORT)
    msdk_printf(MSDK_STRING("   [-rdrm]                   - render decoded data in a thru DRM frame buffer\n"));
    msdk_printf(MSDK_STRING("   [-window x y w h]         - set render window position and size\n"));
#endif
    msdk_printf(MSDK_STRING("   [-low_latency]            - configures decoder for low latency mode (supported only for H.264 and JPEG codec)\n"));
    msdk_printf(MSDK_STRING("   [-calc_latency]           - calculates latency during decoding and prints log (supported only for H.264 and JPEG codec)\n"));
    msdk_printf(MSDK_STRING("   [-async]                  - depth of asynchronous pipeline. default value is 4. must be between 1 and 20\n"));
    msdk_printf(MSDK_STRING("   [-gpucopy::<on,off>] Enable or disable GPU copy mode\n"));
#if !defined(_WIN32) && !defined(_WIN64)
    msdk_printf(MSDK_STRING("   [-threads_num]            - number of mediasdk task threads\n"));
    msdk_printf(MSDK_STRING("   [-threads_schedtype]      - scheduling type of mediasdk task threads\n"));
    msdk_printf(MSDK_STRING("   [-threads_priority]       - priority of mediasdk task threads\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_thread_printf_scheduling_help();
#endif
#if defined(_WIN32) || defined(_WIN64)
    msdk_printf(MSDK_STRING("   [-jpeg_rotate n]          - rotate jpeg frame n degrees \n"));
    msdk_printf(MSDK_STRING("       n(90,180,270)         - number of degrees \n"));

    msdk_printf(MSDK_STRING("\nFeatures: \n"));
    msdk_printf(MSDK_STRING("   Press 1 to toggle fullscreen rendering on/off\n"));
#endif
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Example:\n"));
    msdk_printf(MSDK_STRING("  %s h265 -i in.bit -o out.yuv -p 15dd936825ad475ea34e35f3f54217a6\n"), strAppName);
}

static mfxStatus ParseInputString(msdk_char* strInput[], mfxU8 nArgNum, sInputParams* pParams)
{
    if (1 == nArgNum)
    {
        PrintHelp(strInput[0], NULL);
        return MFX_ERR_UNSUPPORTED;
    }

    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    // set default implementation
    pParams->bUseHWLib = true;
#if defined(LIBVA_SUPPORT)
    pParams->libvaBackend = MFX_LIBVA_DRM;
#endif

    for (mfxU8 i = 1; i < nArgNum; i++)
    {
        if (MSDK_CHAR('-') != strInput[i][0])
        {
            mfxStatus sts = StrFormatToCodecFormatFourCC(strInput[i], pParams->videoType);
            if (sts != MFX_ERR_NONE)
            {
                PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (!IsDecodeCodecSupported(pParams->videoType))
            {
                PrintHelp(strInput[0], MSDK_STRING("Unsupported codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (pParams->videoType == CODEC_MVC)
            {
                pParams->videoType = MFX_CODEC_AVC;
                pParams->bIsMVC = true;
            }
            continue;
        }

        if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-sw")))
        {
            pParams->bUseHWLib = false;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-hw")))
        {
            pParams->bUseHWLib = true;
        }
#if D3D_SURFACES_SUPPORT
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d")))
        {
            pParams->memType = D3D9_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d11")))
        {
            pParams->memType = D3D11_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-r")))
        {
            pParams->mode = MODE_RENDERING;
            // use d3d9 rendering by default
            if (SYSTEM_MEMORY == pParams->memType)
                pParams->memType = D3D9_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-wall")))
        {
            if(i + 6 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -wall key"));
                return MFX_ERR_UNSUPPORTED;
            }
            // use d3d9 rendering by default
            if (SYSTEM_MEMORY == pParams->memType)
                pParams->memType = D3D9_MEMORY;

            pParams->mode = MODE_RENDERING;

            msdk_opt_read(strInput[++i], pParams->nWallW);
            msdk_opt_read(strInput[++i], pParams->nWallH);
            msdk_opt_read(strInput[++i], pParams->nWallCell);
            msdk_opt_read(strInput[++i], pParams->nWallMonitor);

            mfxU32 nTitle;
            msdk_opt_read(strInput[++i], nTitle);

            pParams->bWallNoTitle = 0 == nTitle;

            msdk_opt_read(strInput[++i], pParams->nWallTimeout);
        }
#endif
#if defined(LIBVA_SUPPORT)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-vaapi")))
        {
            pParams->memType = D3D9_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-r")))
        {
            pParams->memType = D3D9_MEMORY;
            pParams->mode = MODE_RENDERING;
            pParams->libvaBackend = MFX_LIBVA_X11;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-rwld")))
        {
            pParams->memType = D3D9_MEMORY;
            pParams->mode = MODE_RENDERING;
            pParams->libvaBackend = MFX_LIBVA_WAYLAND;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-perf")))
        {
            pParams->bPerfMode = true;
        }
        else if (0 == msdk_strncmp(strInput[i], MSDK_STRING("-rdrm"), 5))
        {
            pParams->memType = D3D9_MEMORY;
            pParams->mode = MODE_RENDERING;
            pParams->libvaBackend = MFX_LIBVA_DRM_MODESET;
            if (strInput[i][5]) {
                if (strInput[i][5] != '-') {
                    PrintHelp(strInput[0], MSDK_STRING("unsupported monitor type"));
                    return MFX_ERR_UNSUPPORTED;
                }
                pParams->monitorType = getMonitorType(&strInput[i][6]);
                if (pParams->monitorType >= MFX_MONITOR_MAXNUMBER) {
                    PrintHelp(strInput[0], MSDK_STRING("unsupported monitor type"));
                    return MFX_ERR_UNSUPPORTED;
                }
            } else {
                pParams->monitorType = MFX_MONITOR_AUTO; // that's case of "-rdrm" pure option
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-window")))
        {
            if(i +4 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -window key"));
                return MFX_ERR_UNSUPPORTED;
            }
            msdk_opt_read(strInput[++i], pParams->nRenderWinX);
            msdk_opt_read(strInput[++i], pParams->nRenderWinY);
            msdk_opt_read(strInput[++i], pParams->Width);
            msdk_opt_read(strInput[++i], pParams->Height);

            if (0 == pParams->Width)
                pParams->Width = 320;
            if (0 == pParams->Height)
                pParams->Height = 240;

            pParams->bRenderWin = true;
        }
#endif
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-low_latency")))
        {
            switch (pParams->videoType)
            {
            case MFX_CODEC_HEVC:
            case MFX_CODEC_AVC:
            case MFX_CODEC_JPEG:
            {
                pParams->bLowLat = true;
                if (!pParams->bIsMVC)
                    break;
            }
            default:
            {
                PrintHelp(strInput[0], MSDK_STRING("-low_latency mode is suppoted only for H.264 and JPEG codecs"));
                return MFX_ERR_UNSUPPORTED;
            }
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-jpeg_rotate")))
        {
            if(MFX_CODEC_JPEG != pParams->videoType)
                return MFX_ERR_UNSUPPORTED;

            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -jpeg_rotate key"));
                return MFX_ERR_UNSUPPORTED;
            }

            msdk_opt_read(strInput[++i], pParams->nRotation);
            if((pParams->nRotation != 90)&&(pParams->nRotation != 180)&&(pParams->nRotation != 270))
            {
                PrintHelp(strInput[0], MSDK_STRING("-jpeg_rotate is supported only for 90, 180 and 270 angles"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-calc_latency")))
        {
            switch (pParams->videoType)
            {
            case MFX_CODEC_HEVC:
            case MFX_CODEC_AVC:
            case MFX_CODEC_JPEG:
            {
                pParams->bCalLat = true;
                if (!pParams->bIsMVC)
                    break;
            }
            default:
            {
                PrintHelp(strInput[0], MSDK_STRING("-calc_latency mode is suppoted only for H.264 and JPEG codecs"));
                return MFX_ERR_UNSUPPORTED;
            }
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-async")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -async key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nAsyncDepth))
            {
                PrintHelp(strInput[0], MSDK_STRING("async is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-di")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -di key"));
                return MFX_ERR_UNSUPPORTED;
            }
            msdk_char diMode[4] = {};
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], diMode))
            {
                PrintHelp(strInput[0], MSDK_STRING("deinterlace value is not set"));
                return MFX_ERR_UNSUPPORTED;
            }

            if (0 == msdk_strcmp(diMode, MSDK_CHAR("bob")))
            {
                pParams->eDeinterlace = MFX_DEINTERLACING_BOB;
            }
            else if (0 == msdk_strcmp(diMode, MSDK_CHAR("adi")))
            {
                pParams->eDeinterlace = MFX_DEINTERLACING_ADVANCED;
            }
            else
            {
                PrintHelp(strInput[0], MSDK_STRING("deinterlace value is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::on")))
        {
            pParams->gpuCopy = MFX_GPUCOPY_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::off")))
        {
            pParams->gpuCopy = MFX_GPUCOPY_OFF;
        }
#if !defined(_WIN32) && !defined(_WIN64)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-threads_num")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -threads_num key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nThreadsNum))
            {
                PrintHelp(strInput[0], MSDK_STRING("threads_num is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-threads_schedtype")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -threads_schedtype key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_thread_get_schedtype(strInput[++i], pParams->SchedulingType))
            {
                PrintHelp(strInput[0], MSDK_STRING("threads_schedtype is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-threads_priority")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -threads_priority key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Priority))
            {
                PrintHelp(strInput[0], MSDK_STRING("threads_priority is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
#endif // #if !defined(_WIN32) && !defined(_WIN64)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-f")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -f key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nMaxFPS))
            {
                PrintHelp(strInput[0], MSDK_STRING("rendering frame rate is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-scr:w")))
        {
            if (i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -scr:w key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->scrWidth))
            {
                PrintHelp(strInput[0], MSDK_STRING("screen width rate is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-scr:h")))
        {
            if (i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -scr:h key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->scrHeight))
            {
                PrintHelp(strInput[0], MSDK_STRING("screen height is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-w")))
        {
            if (i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -w key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Width))
            {
                PrintHelp(strInput[0], MSDK_STRING("width is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-h")))
        {
            if (i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -h key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Height))
            {
                PrintHelp(strInput[0], MSDK_STRING("height is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-n")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -n key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nFrames))
            {
                PrintHelp(strInput[0], MSDK_STRING("rendering frame rate is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i420")))
        {
            pParams->fourcc = MFX_FOURCC_NV12;
            pParams->outI420 = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-nv12")))
        {
            pParams->fourcc = MFX_FOURCC_NV12;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-rgb4")))
        {
            pParams->fourcc = MFX_FOURCC_RGB4;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p010")))
        {
            pParams->fourcc = MFX_FOURCC_P010;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-a2rgb10")))
        {
            pParams->fourcc = MFX_FOURCC_A2RGB10;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-path")))
        {
            i++;
#if defined(_WIN32) || defined(_WIN64)
            msdk_char wchar[MSDK_MAX_FILENAME_LEN];
            msdk_opt_read(strInput[i], wchar);
            std::wstring wstr(wchar);
            std::string str(wstr.begin(), wstr.end());

            strcpy_s(pParams->pluginParams.strPluginPath, str.c_str());
#else
            msdk_opt_read(strInput[i], pParams->pluginParams.strPluginPath);
#endif
            pParams->pluginParams.type = MFX_PLUGINLOAD_TYPE_FILE;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i:null")))
        {
            ;
        }
        else // 1-character options
        {
            switch (strInput[i][1])
            {
            case MSDK_CHAR('p'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE == ConvertStringToGuid(strInput[i], pParams->pluginParams.pluginGuid))
                    {
                        pParams->pluginParams.type = MFX_PLUGINLOAD_TYPE_GUID;
                    }
                    else
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Unknown options"));
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-p' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('i'):
                if (++i < nArgNum) {
                    msdk_opt_read(strInput[i], pParams->strSrcFile);
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-i' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('o'):
                if (++i < nArgNum) {
                    pParams->mode = MODE_FILE_DUMP;
                    msdk_opt_read(strInput[i], pParams->strDstFile);
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-o' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('?'):
                PrintHelp(strInput[0], NULL);
                return MFX_ERR_UNSUPPORTED;
            default:
            {
                std::basic_stringstream<msdk_char> stream;
                stream << MSDK_STRING("Unknown option: ") << strInput[i];
                PrintHelp(strInput[0], stream.str().c_str());
                return MFX_ERR_UNSUPPORTED;
            }
            }
        }
    }

    if (0 == msdk_strlen(pParams->strSrcFile) && MFX_CODEC_CAPTURE != pParams->videoType)
    {
        msdk_printf(MSDK_STRING("error: source file name not found"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_CAPTURE == pParams->videoType)
    {
        if (!pParams->scrWidth || !pParams->scrHeight)
        {
            msdk_printf(MSDK_STRING("error: for screen capture, width and height must be specified manually (-w and -h)"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (pParams->scrWidth || pParams->scrHeight)
    {
        msdk_printf(MSDK_STRING("error: width and height parameters are supported only by screen capture decoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->mode == MODE_FILE_DUMP) && (0 == msdk_strlen(pParams->strDstFile)))
    {
        msdk_printf(MSDK_STRING("error: destination file name not found"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_MPEG2   != pParams->videoType &&
            MFX_CODEC_AVC     != pParams->videoType &&
            MFX_CODEC_HEVC    != pParams->videoType &&
            MFX_CODEC_VC1     != pParams->videoType &&
            MFX_CODEC_JPEG    != pParams->videoType &&
            MFX_CODEC_CAPTURE != pParams->videoType &&
            CODEC_VP8         != pParams->videoType)
    {
        PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (pParams->nAsyncDepth == 0)
    {
        pParams->nAsyncDepth = 4; //set by default;
    }

    return MFX_ERR_NONE;
}

using namespace std;
#if defined(_WIN32) || defined(_WIN64)
int _tmain(int argc, TCHAR *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    sInputParams        Params;   // input parameters from command line
    CDecodingPipeline   Pipeline; // pipeline for decoding, includes input file reader, decoder and output file writer

    mfxStatus sts = MFX_ERR_NONE; // return value check

    sts = ParseInputString(argv, (mfxU8)argc, &Params);
    MSDK_CHECK_PARSE_RESULT(sts, MFX_ERR_NONE, 1);
    {
        cout <<"videoType" <<'\t' <<Params.videoType<< endl; // MFX_CODEC_AVC
        cout << "mode " << '\t' << Params.mode<<endl;
        cout << "memType " <<'\t' << Params.memType<<endl;
        cout <<"bUseHWLib"<< '\t' << Params.bUseHWLib<<endl; // true if application wants to use HW mfx library
        cout << "bIsMVC" <<'\t' << Params.bIsMVC<<endl; // true if Multi-View Codec is in use
        cout << "bLowLat" <<'\t' << Params.bLowLat<<endl; // low latency mode
        cout << "bCalLat " <<'\t' << Params.bCalLat<<endl; // latency calculation
        cout << "nMaxFPS "<<'\t' << Params.nMaxFPS<<endl; //rendering limited by certain fps
        cout <<"nWallCell " <<'\t' << Params.nWallCell<<endl;
        cout <<"nWallW " <<'\t' << Params.nWallW<<endl; //number of windows located in each row
        cout <<"nWallH< " <<'\t' << Params.nWallH<<endl; //number of windows located in each column
        cout <<"nWallMonitor " <<'\t' << Params.nWallMonitor<<endl; //monitor id, 0,1,.. etc
        cout <<"bWallNoTitle " <<'\t' << Params.bWallNoTitle<<endl; //whether to show title for each window with fps value
        cout <<"nWallTimeout " <<'\t' << Params.nWallTimeout<<endl; //timeout for -wall option
        cout <<"numViews " <<'\t' << Params.numViews<<endl; // number of views for Multi-View Codec
        cout <<"nRotation " <<'\t' << Params.nRotation<<endl; // rotation for Motion JPEG Codec
        cout <<"nAsyncDepth " <<'\t' << Params.nAsyncDepth<<endl; // asyncronous queue
        cout <<"gpuCopy " <<'\t' << Params.gpuCopy<<endl; // GPU Copy mode (three-state option)
        cout <<"nThreadsNum " <<'\t' << Params.nThreadsNum<<endl;
        cout << "SchedulingType" << Params.SchedulingType<<endl;
        cout << "Priority" << Params.Priority<<endl;

        cout << " scrWidth"<<'\t' << Params.scrWidth<<endl;
        cout << " scrHeight"<<'\t' << Params.scrHeight<<endl;

        cout <<" Width" <<'\t' << Params.Width<<endl;
        cout << " Height"<<'\t' << Params.Height<<endl;

        cout <<" fourcc" <<'\t' << Params.fourcc<<endl;
        cout << "nFrames "<<'\t' << Params.nFrames<<endl;
        cout << " eDeinterlace"<<'\t' << Params.eDeinterlace<<endl;
        cout <<" outI420" <<'\t' << Params.outI420<<endl;

        cout <<"bPerfMode " <<'\t' << Params.bPerfMode<<endl;
        cout <<" bRenderWin" <<'\t' << Params.bRenderWin<<endl;
        cout <<" nRenderWinX" <<'\t' << Params.nRenderWinX<<endl;
        cout <<" nRenderWinY" <<'\t' << Params.nRenderWinY<<endl;

        cout << "monitorType \t" <<  Params.monitorType<<endl;
#if defined(LIBVA_SUPPORT)
        cout << "libvaBackend \t" << Params.libvaBackend<<endl;
#endif // defined(MFX_LIBVA_SUPPORT)

        cout << "strSrcFile \t" << Params.strSrcFile<<endl;
        cout << "strDstFile \t" << Params.strDstFile<<endl;
    }
    if (Params.bIsMVC)
        Pipeline.SetMultiView();

    sts = Pipeline.Init(&Params);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);

    // print stream info
    Pipeline.PrintInfo();

    msdk_printf(MSDK_STRING("Decoding started\n"));

    for (;;)
    {
        sts = Pipeline.RunDecoding();

        if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts || MFX_ERR_DEVICE_LOST == sts || MFX_ERR_DEVICE_FAILED == sts)
        {
            if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts)
            {
                msdk_printf(MSDK_STRING("\nERROR: Incompatible video parameters detected. Recovering...\n"));
            }
            else
            {
                msdk_printf(MSDK_STRING("\nERROR: Hardware device was lost or returned unexpected error. Recovering...\n"));
                sts = Pipeline.ResetDevice();
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
            }

            sts = Pipeline.ResetDecoder(&Params);
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
            continue;
        }
        else
        {
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, 1);
            break;
        }
    }

    msdk_printf(MSDK_STRING("\nDecoding finished\n"));

    return 0;
}
#endif
