/////////////////////////////////////////////////////////////////////////////////////////////
// Project:TsFormat
// Author:
// Date:星期一, 三月 5, 2012
// Description:The Ts Muxer of Dom Frame,Current Only for Video(h.264) and Audio(aac)
/////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <cassert>
#include "TsMuxer.h"
#include "BitWriter.h"


#define _PCRBASE_ 300

//TsMuxFormat
CTsMuxer::CTsMuxer(bool is_mp3, bool is_265)
{
    _is_mp3 = is_mp3;
    _is_265 = is_265;
    memset(StuffingBytes,0xFF,sizeof(StuffingBytes));
    memset(m_nContinuityCounter,0x0,sizeof(m_nContinuityCounter));

    m_lpVideoBuffer = NULL;
    m_llLastDts = 0;

    m_bHasVideo = false;
    m_bHasAudio = false;

    m_dwAudioBaseIndex = AUDIO_STREAM_BASE_ID;
    m_dwVideoBaseIndex = VIDEO_STREAM_BASE_ID;

    m_bVideoFirst = true;
    m_bAudioFirst = true;


#ifdef _LOG_VIDEO_STREAM
    m_hVideoFile = CreateFile(_T("d:\\media\\ok.h264"),GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    m_hAudioFile = CreateFile(_T("d:\\media\\ok.aac"),GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
#endif
}

CTsMuxer::~CTsMuxer()
{
    if(m_lpVideoBuffer)
        delete [] m_lpVideoBuffer;
#ifdef _LOG_VIDEO_STREAM
    CloseHandle(m_hVideoFile);
    CloseHandle(m_hAudioFile);
#endif
}




HRESULT CTsMuxer::OnWriteFrame(IMediaFrame* pFrame, bool force_Pat_pmt)
{
    HRESULT hr = S_OK;
    DWORD dwStreamID = pFrame->strmID;
    IMediaFrame::MMEDIA_FRAME_INFO info = pFrame->info;
    LPBYTE lpData = pFrame->lpdata;
    DWORD dwSize = pFrame->dwSize;


    LONGLONG llDts = info.dts;
    LONGLONG llPts = info.pts;


    if(m_bHasVideo && (dwStreamID == VIDEO_STREAM_BASE_ID))
    {

        if(m_bVideoFirst)
        {
            m_bVideoFirst = false;
        }
        llDts = info.dts ;
        llPts = info.pts ;
        if(info.dts - m_llLastDts > 90 * 1000 || (true == force_Pat_pmt))
        {
            WritePAT();
            WritePMT(m_bHasVideo?CTVIT_PID_VIDEO:0,m_bHasAudio?CTVIT_PID_AUDIO:0);
            m_llLastDts = info.dts;
        }
#ifdef _LOG_VIDEO_STREAM
        DWORD dwWrite;
        WriteFile(m_hVideoFile,lpData,dwSize,&dwWrite,NULL);
#endif
        WriteVideoPacket(lpData,dwSize,llDts,llPts);
    }
    else if(m_bHasAudio && (dwStreamID == AUDIO_STREAM_BASE_ID))
    {

        if(m_bAudioFirst)
        {
            //m_llAudioTimeBase = info.dts;
            m_bAudioFirst = false;
        }
        llDts = info.dts ;
        llPts = info.pts ;
        //static INT nCount = 0;
        //TCHAR szLog[MAX_PATH] = {0};
        //sprintf(szLog,"pts = %I64d,dts = %I64d\r\n",info.pts,info.dts);
        //OutputDebugString(szLog);
        //LOG(_T("pts = %I64d,dts = %I64d"),info.pts,info.dts);
#ifdef _LOG_VIDEO_STREAM
        DWORD dwWrite;
        WriteFile(m_hAudioFile,lpData,dwSize,&dwWrite,NULL);
#endif
        WriteAudioPacket(lpData,dwSize,llDts,llPts);
    }
    return hr;
}


HRESULT CTsMuxer::InternalOpen(muxer_sink* s, bool hasV, bool hasA)
{
    HRESULT hr = S_OK;
    //JCHK(m_pStream,E_FAIL);
    //Add......
    //CDomPtr<IMediaLoad> spML;
    //m_pStream->QueryInterface(IID_IMediaLoad,
    //	(LPVOID*)&spML);
    //std::string strName = spML->GetUrl();
    //JCHK0(m_handlers->size() == 2, E_FAIL,_T("多流不支持"));
    m_lpVideoBuffer = new BYTE[MAX_PAYLOAD_SIZE];
    if(m_lpVideoBuffer == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;



    m_MemoryCache.SetSink(s);
    m_MemoryCache.SetSize(188 * 1024 * 20);

//	HandlerIt itr;
//	int nVideoID = 2;
//	for (itr = m_handlers->begin(); itr != m_handlers->end(); ++itr)
//	{
//		CMediaHandler* strm = *itr;
//		CMMediaType mt;
//		JIF(strm->GetMediaType(&mt));
		
//		switch(mt.type & MMEDIA_TYPE_MAJOR_MASK)
//		{
//		case MMEDIA_TYPE_VIDEO:
//			{
                m_bHasVideo = hasV;
//				strm->SetID(m_dwVideoBaseIndex++);
//				MMEDIA_VIDEO_TYPE* pMvt;
//				mt.GetMajType(pMvt);
//				if(mt.type == MMEDIA_VIDEO_TYPE_H264)
//				{
//					MEDIA_VIDEO_SUB_TYPE_H264* pH264;
//					mt.GetSubType(pH264);
//				}
//			}
//			break;
//		case  MMEDIA_TYPE_AUDIO:
//			{
                m_bHasAudio = hasA;
//				strm->SetID(m_dwAudioBaseIndex);
//				MMEDIA_AUDIO_TYPE* pMat;
//				mt.GetMajType(pMat);
//				m_dwChannelCount = pMat->channel;
//				m_dwFrequency = pMat->samplePerSec;
//			}
//			break;
//		default:
//			return E_UNEXPECTED;
//		}
//	}

    WritePAT();
    WritePMT(m_bHasVideo?CTVIT_PID_VIDEO:0,m_bHasAudio?CTVIT_PID_AUDIO:0);

    return hr;
}

HRESULT CTsMuxer::InternalClose()
{
    HRESULT hr = S_OK;

    m_MemoryCache.Close();

    if(m_lpVideoBuffer)
    {
        delete [] m_lpVideoBuffer;
        m_lpVideoBuffer = NULL;
    }
    return hr;
}

BYTE CTsMuxer::StuffingBytes[CTVIT_MPEG2TS_PACKET_SIZE];

DWORD const CTsMuxer::CRC_Table[256] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
    0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
    0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
    0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
    0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
    0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
    0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
    0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
    0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
    0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
    0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
    0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
    0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
    0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
    0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
    0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
    0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
    0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
    0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
    0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
    0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
    0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

DWORD CTsMuxer::ComputeCRC(const BYTE* lpData,DWORD dwSize)
{
    DWORD dwCrc = 0xFFFFFFFF;

    for (DWORD i = 0;i < dwSize;i++) {
        dwCrc = (dwCrc << 8) ^ CRC_Table[((dwCrc >> 24) ^ *lpData++) & 0xFF];
    }

    return dwCrc;
}

bool CTsMuxer::WriteTsHeader(DWORD dwPid,DWORD dwCCIndex,bool bPayloadStart,DWORD& dwPayloadSize,bool bWithPCR,UINT64 nPCR)
{
    unsigned char header[4];
    header[0] = CTVIT_MPEG2TS_SYNC_BYTE;
    header[1] = ((bPayloadStart ? 1 : 0) << 6) | (dwPid >> 8);
    header[2] = dwPid & 0xFF;

    DWORD dwAdaptationSize = 0;
    if (bWithPCR) dwAdaptationSize += 2 + CTVIT_MPEG2TS_PCR_ADAPTATION_SIZE;

    if (dwPayloadSize + dwAdaptationSize > CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE) {
        dwPayloadSize = CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE - dwAdaptationSize;
    }

    if (dwAdaptationSize + dwPayloadSize < CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE) {
        dwAdaptationSize = CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE - dwPayloadSize;
    }

    if (dwAdaptationSize == 0) {
        header[3] = (1 << 4) | ((m_nContinuityCounter[dwCCIndex]++) & 0x0F);

        m_MemoryCache.Write(header,4);

    } else {
        header[3] = (3 << 4) | ((m_nContinuityCounter[dwCCIndex]++) & 0x0F);
        m_MemoryCache.Write(header,4);
        if (dwAdaptationSize == 1) {
            BYTE byStuffing = 0;
            m_MemoryCache.Write(&byStuffing,1);
        } else {
            BYTE bTemp = dwAdaptationSize - 1;
            m_MemoryCache.Write(&bTemp,1);
            bTemp = bWithPCR ? (1 << 4) : 0;
            m_MemoryCache.Write(&bTemp,1);
            unsigned int pcr_size = 0;
            if (bWithPCR) {
                pcr_size = CTVIT_MPEG2TS_PCR_ADAPTATION_SIZE;
                UINT64 pcr_base = nPCR / _PCRBASE_;
                UINT32 pcr_ext  = nPCR % _PCRBASE_;
                CBitWriter writer(pcr_size);
                writer.Write(pcr_base >> 32, 1);
                writer.Write(pcr_base, 32);
                writer.Write(0x3F, 6);
                writer.Write(pcr_ext, 9);
                m_MemoryCache.Write((LPBYTE)writer.GetData(),pcr_size);
            }
            if (dwAdaptationSize > 2) {
                m_MemoryCache.Write(StuffingBytes,dwAdaptationSize - pcr_size - 2);
            }
        }
    }
    return true;
}

bool CTsMuxer::WritePAT()
{
    DWORD dwPayloadSize = CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE;
    WriteTsHeader(0,CTVIT_CC_INDEX_PAT,true,dwPayloadSize,false,0);

    CBitWriter writer(1024);

    writer.Write(0, 8);
    writer.Write(0, 8);
    writer.Write(1, 1);
    writer.Write(0, 1);
    writer.Write(3, 2);
    writer.Write(13, 12);
    writer.Write(1, 16);
    writer.Write(3, 2);
    writer.Write(0, 5);
    writer.Write(1, 1);
    writer.Write(0, 8);
    writer.Write(0, 8);
    writer.Write(1, 16);
    writer.Write(7, 3);
    writer.Write(CTVIT_PID_PMT, 13);
    writer.Write(ComputeCRC(writer.GetData() + 1, 17 - 1 - 4), 32);


    m_MemoryCache.Write((LPBYTE)writer.GetData(),17);
    m_MemoryCache.Write(StuffingBytes,CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE - 17);

    return true;
}
                                                                         
bool CTsMuxer::WritePMT(DWORD dwVideoID,DWORD dwAudioID)
{
    if(!m_bHasVideo && !m_bHasAudio)
        return false;

    DWORD dwPlayloadSize = CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE;
    WriteTsHeader(CTVIT_PID_PMT,CTVIT_CC_INDEX_PMT,true,dwPlayloadSize,false,0);

    CBitWriter writer(1024);

    unsigned int section_length = 13;
    unsigned int pcr_pid = 0;

    if (dwAudioID) {
        section_length += 5;
        pcr_pid = dwAudioID;
    }

    if (dwVideoID) {
        section_length += 5;
        pcr_pid = dwVideoID;
    }
                                                                                                                                               
    writer.Write(0, 8);
    writer.Write(2, 8);
    writer.Write(1, 1);
    writer.Write(0, 1);
    writer.Write(3, 2);
    writer.Write(section_length, 12);
    writer.Write(1, 16);
    writer.Write(3, 2);
    writer.Write(0, 5);
    writer.Write(1, 1);
    writer.Write(0, 8);
    writer.Write(0, 8);
    writer.Write(7, 3);
    writer.Write(pcr_pid, 13);
    writer.Write(0xF, 4);
    writer.Write(0, 12);

    if (dwAudioID) {
        if (false == _is_mp3)
            writer.Write(0x0F, 8); // AAC
        else
            writer.Write(0x03, 8); // MP2(MPEG1 layter I/II/III)
        writer.Write(0x7, 3);
        writer.Write(dwAudioID, 13);
        writer.Write(0xF, 4);
        writer.Write(0, 12);
    }

                                                                                                                                         
    if (dwVideoID) {
        if (false == _is_265)
            writer.Write(0x1B, 8); // AVC
        //writer.Write(0x2, 8); // H.262 or 11172-2
        else
            writer.Write(0x24, 8); // HEVC
        writer.Write(0x7, 3);
        writer.Write(dwVideoID, 13);
        writer.Write(0xF, 4);
        writer.Write(0, 12);
    }
                                                                                                                                              
    writer.Write(ComputeCRC(writer.GetData() + 1, section_length - 1), 32);

    m_MemoryCache.Write((LPBYTE)writer.GetData(),section_length + 4);
    m_MemoryCache.Write(StuffingBytes,CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE - (section_length + 4));

    return true;
}
                                                                                                                                                  
bool CTsMuxer::WriteVideoPacket(LPBYTE lpData, DWORD dwSize,LONGLONG llDts,LONGLONG llPts)
{
    if(dwSize + 6 > MAX_PAYLOAD_SIZE)
    {
        delete [] m_lpVideoBuffer;
        m_lpVideoBuffer = new BYTE[dwSize + MAX_PAYLOAD_SIZE];
    }

    if (this->_is_265)
    {
        return WritePESPacket(CTVIT_PID_VIDEO,CTVIT_CC_INDEX_VIDEO,CTVIT_STREAM_ID_VIDEO,lpData,dwSize,llDts,true,llPts,true);
    }
    if(*(LPDWORD)lpData == 0x01000000 && lpData[4] == 0x9) // AUD
    {
        return WritePESPacket(CTVIT_PID_VIDEO,CTVIT_CC_INDEX_VIDEO,CTVIT_STREAM_ID_VIDEO,lpData,dwSize,llDts,true,llPts,true);
    }
    else
    {
        // for apple, add AUD to stream
        m_lpVideoBuffer[0] = 0;
        m_lpVideoBuffer[1] = 0;
        m_lpVideoBuffer[2] = 0;
        m_lpVideoBuffer[3] = 1;
        m_lpVideoBuffer[4] = 9;
        m_lpVideoBuffer[5] = 0xE0;
        memcpy(&m_lpVideoBuffer[6],lpData,dwSize);
        dwSize += 6;
        WritePESPacket(CTVIT_PID_VIDEO,CTVIT_CC_INDEX_VIDEO,CTVIT_STREAM_ID_VIDEO,m_lpVideoBuffer,dwSize,llDts,true,llPts,true);
    }

    return true;
}

bool CTsMuxer::WriteAudioPacket(LPBYTE lpData, DWORD dwSize,LONGLONG llDts,LONGLONG llPts)
{
    return WritePESPacket(CTVIT_PID_AUDIO,CTVIT_CC_INDEX_AUDIO,CTVIT_STREAM_ID_AUDIO,lpData,dwSize,llDts,true,llPts,!m_bHasVideo);
}

bool CTsMuxer::WritePESPacket(DWORD dwPid,DWORD dwCCIndex,DWORD dwStreamID,LPBYTE lpData,DWORD dwSize,LONGLONG llDts,bool bWithDts,LONGLONG llPts,bool bWithPCR)
{
    DWORD dwPesHeaderSize = 14 + (bWithDts ? 5 : 0);
    CBitWriter pes_header(dwPesHeaderSize);

    pes_header.Write(0x000001, 24);
    pes_header.Write(dwStreamID, 8);
    pes_header.Write(dwStreamID == CTVIT_STREAM_ID_VIDEO ? 0 : (dwSize + dwPesHeaderSize - 6),16);
    pes_header.Write(2, 2);
    pes_header.Write(0, 2);
    pes_header.Write(0, 1);
    pes_header.Write(1, 1);
    pes_header.Write(0, 1);
    pes_header.Write(0, 1);
    pes_header.Write(bWithDts ? 3 : 2, 2);
    pes_header.Write(0, 1);
    pes_header.Write(0, 1);
    pes_header.Write(0, 1);
    pes_header.Write(0, 1);
    pes_header.Write(0, 1);
    pes_header.Write(0, 1);
    pes_header.Write(dwPesHeaderSize - 9, 8);

    pes_header.Write(bWithDts ? 3 : 2, 4);
    pes_header.Write(llPts >> 30, 3);
    pes_header.Write(1, 1);
    pes_header.Write(llPts >> 15, 15);
    pes_header.Write(1, 1);
    pes_header.Write(llPts, 15);
    pes_header.Write(1, 1);

    if (bWithDts) {
        pes_header.Write(1, 4);
        pes_header.Write(llDts >> 30, 3);
        pes_header.Write(1, 1);
        pes_header.Write(llDts >> 15, 15);
        pes_header.Write(1, 1);
        pes_header.Write(llDts, 15);
        pes_header.Write(1, 1);
    }

    bool bFirstPacket = true;
    dwSize += dwPesHeaderSize;
    while (dwSize) {
        DWORD dwPayloadSize = dwSize;
        if (dwPayloadSize > CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE) dwPayloadSize = CTVIT_MPEG2TS_PACKET_PAYLOAD_SIZE;

        if (bFirstPacket)  {
            WriteTsHeader(dwPid,dwCCIndex,bFirstPacket,dwPayloadSize,bWithPCR,(bWithDts ? llDts : llPts) * _PCRBASE_);
            bFirstPacket = false;

            m_MemoryCache.Write((LPBYTE)pes_header.GetData(),dwPesHeaderSize);
            m_MemoryCache.Write(lpData,dwPayloadSize - dwPesHeaderSize);

            lpData += dwPayloadSize - dwPesHeaderSize;
        } else {
            WriteTsHeader(dwPid,dwCCIndex,bFirstPacket,dwPayloadSize,false,0);

            m_MemoryCache.Write(lpData,dwPayloadSize);

            lpData += dwPayloadSize;
        }
        dwSize -= dwPayloadSize;
    }

    return true;
}

