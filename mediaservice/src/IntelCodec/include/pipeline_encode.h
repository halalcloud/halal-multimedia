/******************************************************************************\
Copyright (c) 2005-2016, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
\**********************************************************************************/

#ifndef __PIPELINE_ENCODE_H__
#define __PIPELINE_ENCODE_H__

#include "sample_defs.h"
#include "hw_device.h"

#ifdef D3D_SURFACES_SUPPORT
#pragma warning(disable : 4201)
#endif

#include "sample_utils.h"
#include "sample_params.h"
#include "base_allocator.h"
#include "time_statistics.h"

#include "mfxmvc.h"
#include "mfxvideo.h"
#include "mfxvp8.h"
#include "mfxvideo++.h"
#include "mfxplugin.h"
#include "mfxplugin++.h"

#include <vector>
#include <memory>

#include "plugin_loader.h"

#if defined (ENABLE_V4L2_SUPPORT)
#include "v4l2_util.h"
#endif

msdk_tick time_get_tick(void);
msdk_tick time_get_frequency(void);

enum {
    MVC_DISABLED          = 0x0,
    MVC_ENABLED           = 0x1,
    MVC_VIEWOUTPUT        = 0x2,    // 2 output bitstreams
};

enum MemType {
    SYSTEM_MEMORY = 0x00,
    D3D9_MEMORY   = 0x01,
    D3D11_MEMORY  = 0x02,
};

struct sInputParams
{
    mfxU16 nTargetUsage;
    mfxU32 CodecId;
    mfxU32 ColorFormat;
    mfxU16 nPicStruct;
    mfxU16 nWidth; // source picture width
    mfxU16 nHeight; // source picture height
    mfxF64 dFrameRate;
    mfxU32 nNumFrames;
    mfxU16 nBitRate;
    mfxU16 MVC_flags;
    mfxU16 nGopPicSize;
    mfxU16 nGopRefDist;
    mfxU16 nNumRefFrame;
    mfxU16 nBRefType;
    mfxU16 nIdrInterval;
    mfxU16 reserved[4];

    mfxU16 nQuality; // quality parameter for JPEG encoder

    mfxU32 numViews; // number of views for Multi-View Codec

    mfxU16 nDstWidth; // destination picture width, specified if resizing required
    mfxU16 nDstHeight; // destination picture height, specified if resizing required

    MemType memType;
    bool bUseHWLib; // true if application wants to use HW MSDK library

    msdk_char strSrcFile[MSDK_MAX_FILENAME_LEN];

    sPluginParams pluginParams;

    std::vector<msdk_char*> srcFileBuff;
    std::vector<msdk_char*> dstFileBuff;

    mfxU32  HEVCPluginVersion;
    mfxU8 nRotationAngle; // if specified, enables rotation plugin in mfx pipeline
    msdk_char strPluginDLLPath[MSDK_MAX_FILENAME_LEN]; // plugin dll path and name

    mfxU16 nAsyncDepth; // depth of asynchronous pipeline, this number can be tuned to achieve better performance
    mfxU16 gpuCopy; // GPU Copy mode (three-state option)

    mfxU16 nRateControlMethod;
    mfxU16 nLADepth; // depth of the look ahead bitrate control  algorithm
    mfxU16 nMaxSliceSize; //maximum size of slice
    mfxU16 nQPI;
    mfxU16 nQPP;
    mfxU16 nQPB;

    mfxU16 nNumSlice;
    bool UseRegionEncode;

    bool isV4L2InputEnabled;

#if defined (ENABLE_V4L2_SUPPORT)
    msdk_char DeviceName[MSDK_MAX_FILENAME_LEN];
    msdk_char MipiModeName[MSDK_MAX_FILENAME_LEN];
    enum V4L2PixelFormat v4l2Format;
    int MipiPort;
    enum AtomISPMode MipiMode;
#endif

};

struct outter
{
    bool isKeyFrame;
    int64_t timeStamp;
    int64_t dts;
    uint8_t* pData;
    uint32_t len;
    uint32_t used;
    outter(uint32_t len = 0)
        :len(len), used(0), isKeyFrame(false)
        ,timeStamp(0)
    {
        pData = new uint8_t[len];
    }
    int write(uint8_t* p, uint32_t len)
    {
        if (nullptr == p
                || len == 0
                || len > this->len)
            return -1;
        memcpy(pData, p, len);
        used = len;
    }
    void reset(uint32_t len)
    {
        if (this->len < len)
        {
            delete [] pData;
            pData = new uint8_t[len];
            this->len = len;
        }
        used = 0;
    }
};
struct sTask
{
    mfxBitstream mfxBS;
    mfxSyncPoint EncSyncP;
    std::list<mfxSyncPoint> DependentVppTasks;
    CSmplBitstreamWriter *pWriter;

    sTask();
    mfxStatus WriteBitstream(outter &otr);
    mfxStatus Reset();
    mfxStatus Init(mfxU32 nBufferSize, CSmplBitstreamWriter *pWriter = NULL);
    mfxStatus Close();
};


class CEncTaskPool
{
public:
    CEncTaskPool();
    virtual ~CEncTaskPool();

    virtual mfxStatus Init(MFXVideoSession* pmfxSession, CSmplBitstreamWriter* pWriter, mfxU32 nPoolSize, mfxU32 nBufferSize, CSmplBitstreamWriter *pOtherWriter = NULL);
    virtual mfxStatus GetFreeTask(sTask **ppTask);
    virtual mfxStatus SynchronizeFirstTask(outter &otr);

    virtual CTimeStatistics& GetOverallStatistics() { return m_statOverall;}
    virtual CTimeStatistics& GetFileStatistics() { return m_statFile;}
    virtual void Close();
protected:
    sTask* m_pTasks;
    mfxU32 m_nPoolSize;
    mfxU32 m_nTaskBufferStart;

    MFXVideoSession* m_pmfxSession;

    CTimeStatistics m_statOverall;
    CTimeStatistics m_statFile;
    virtual mfxU32 GetFreeTaskIndex();
};

/* This class implements a pipeline with 2 mfx components: vpp (video preprocessing) and encode */
class CEncodingPipeline
{
public:
    struct input
    {
        uint8_t*y,*u,*v;
        uint32_t stride;
        int64_t pts;
    };

    CEncodingPipeline();
    virtual ~CEncodingPipeline();

    virtual mfxStatus Init(sInputParams *pParams);
    virtual mfxStatus Run(outter &otr, input *data = nullptr);
    virtual mfxStatus Flush(outter &otr);
    mfxStatus Flush2(outter &otr);
    virtual void Close();
    virtual mfxStatus ResetMFXComponents(sInputParams* pParams);
    virtual mfxStatus ResetDevice();

    void SetMultiView();
    void SetNumView(mfxU32 numViews) { m_nNumView = numViews; }
    virtual void  PrintInfo();

    void InitV4L2Pipeline(sInputParams *pParams);
    mfxStatus CaptureStartV4L2Pipeline();
    void CaptureStopV4L2Pipeline();

#if defined (ENABLE_V4L2_SUPPORT)
    v4l2Device v4l2Pipeline;
    pthread_t m_PollThread;
#endif

    mfxStatus encode(sTask *pCurrentTask, mfxU16 nEncSurfIdx);
    mfxStatus getSurface(mfxFrameSurface1 *pSurf, input *data = nullptr);

protected:
    std::pair<CSmplBitstreamWriter *,CSmplBitstreamWriter *> m_FileWriters;
    CSmplYUVReader m_FileReader;
    CEncTaskPool m_TaskPool;

    MFXVideoSession m_mfxSession;
    MFXVideoENCODE* m_pmfxENC;
    MFXVideoVPP* m_pmfxVPP;

    mfxVideoParam m_mfxEncParams;
    mfxVideoParam m_mfxVppParams;

    mfxU16 m_MVCflags; // MVC codec is in use

    std::auto_ptr<MFXVideoUSER> m_pUserModule;
    std::auto_ptr<MFXPlugin> m_pPlugin;

    MFXFrameAllocator* m_pMFXAllocator;
    mfxAllocatorParams* m_pmfxAllocatorParams;
    MemType m_memType;
    bool m_bExternalAlloc; // use memory allocator as external for Media SDK

    mfxFrameSurface1* m_pEncSurfaces; // frames array for encoder input (vpp output)
    mfxFrameSurface1* m_pVppSurfaces; // frames array for vpp input
    mfxFrameAllocResponse m_EncResponse;  // memory allocation response for encoder
    mfxFrameAllocResponse m_VppResponse;  // memory allocation response for vpp

    mfxU32 m_nNumView;

    mfxU32 m_nFramesToProcess; // number of frames to process

    // for disabling VPP algorithms
    mfxExtVPPDoNotUse m_VppDoNotUse;
    // for MVC encoder and VPP configuration
    mfxExtMVCSeqDesc m_MVCSeqDesc;
    mfxExtCodingOption m_CodingOption;
    // for look ahead BRC configuration
    mfxExtCodingOption2 m_CodingOption2;
    // HEVC
    mfxExtHEVCParam m_ExtHEVCParam;

    // external parameters for each component are stored in a vector
    std::vector<mfxExtBuffer*> m_VppExtParams;
    std::vector<mfxExtBuffer*> m_EncExtParams;

    CHWDevice *m_hwdev;

    bool isV4L2InputEnabled;

    CTimeStatistics m_statOverall;
    CTimeStatistics m_statFile;
    virtual mfxStatus InitMfxEncParams(sInputParams *pParams);
    virtual mfxStatus InitMfxVppParams(sInputParams *pParams);

    virtual mfxStatus InitFileWriters(sInputParams *pParams);
    virtual void FreeFileWriters();
    virtual mfxStatus InitFileWriter(CSmplBitstreamWriter **ppWriter, const msdk_char *filename);

    virtual mfxStatus AllocAndInitVppDoNotUse();
    virtual void FreeVppDoNotUse();

    virtual mfxStatus AllocAndInitMVCSeqDesc();
    virtual void FreeMVCSeqDesc();

    virtual mfxStatus CreateAllocator();
    virtual void DeleteAllocator();

    virtual mfxStatus CreateHWDevice();
    virtual void DeleteHWDevice();

    virtual mfxStatus AllocFrames();
    virtual void DeleteFrames();

    virtual mfxStatus AllocateSufficientBuffer(mfxBitstream* pBS);

    virtual mfxStatus GetFreeTask(sTask **ppTask, outter &otr);
    virtual MFXVideoSession& GetFirstSession(){return m_mfxSession;}
    virtual MFXVideoENCODE* GetFirstEncoder(){return m_pmfxENC;}
private:
    mfxStatus encode(mfxStatus sts, sTask *pCurrentTask, mfxU16 nEncSurfIdx);
    FILE* m_fSource;
};

#endif // __PIPELINE_ENCODE_H__
