#include "TsDemuxer.h"
#include <iostream>
#include <cassert>
#include "m3u8parser.h"
class AACFrameSpliter
{
    static const int AP4_SUCCESS = 0;
    static const int AP4_FAILURE = -1;
    class AdtsHeader {
    public:
        // constructor
        AdtsHeader(const uint8_t* bytes)
        {
            // fixed part
            m_Id                     = ( bytes[1] & 0x08) >> 3;
            m_ProtectionAbsent       =   bytes[1] & 0x01;
            m_ProfileObjectType      = ( bytes[2] & 0xC0) >> 6;
            m_SamplingFrequencyIndex = ( bytes[2] & 0x3C) >> 2;
            m_ChannelConfiguration   = ((bytes[2] & 0x01) << 2) |
                    ((bytes[3] & 0xC0) >> 6);
            // variable part
            m_FrameLength = ((unsigned int)(bytes[3] & 0x03) << 11) |
                    ((unsigned int)(bytes[4]       ) <<  3) |
                    ((unsigned int)(bytes[5] & 0xE0) >>  5);
            m_RawDataBlocks =               bytes[6] & 0x03;
        }

        // methods
        int Check()
        {
            // check that the sampling frequency index is valid
            if (m_SamplingFrequencyIndex >= 0xD) {
                return AP4_FAILURE;
            }

            /* MPEG2 does not use all profiles */
            if (m_Id == 1 && m_ProfileObjectType == 3) {
                return AP4_FAILURE;
            }

            return AP4_SUCCESS;
        }

        // members

        // fixed part
        unsigned int m_Id;
        unsigned int m_ProtectionAbsent;
        unsigned int m_ProfileObjectType;
        unsigned int m_SamplingFrequencyIndex;
        unsigned int m_ChannelConfiguration;

        // variable part
        unsigned int m_FrameLength;
        unsigned int m_RawDataBlocks;

        // class methods
        static bool MatchFixed(unsigned char* a, unsigned char* b)
        {
            if (a[0]         ==  b[0] &&
                    a[1]         ==  b[1] &&
                    a[2]         ==  b[2] &&
                    (a[3] & 0xF0) == (b[3] & 0xF0)) {
                return true;
            } else {
                return false;
            }
        }
    };
    static const int ADTS_HEADER_SIZE = 7;
    static const int  ADTS_SYNC_MASK =    0xFFF6; /* 12 sync bits plus 2 layer bits */
    static const int ADTS_SYNC_PATTERN = 0xFFF0; /* 12 sync bits=1 layer=0         */
    uint32_t GetSamplingFrequency(uint32_t idx)
    {
        switch (idx) {
        case 0: return 96000;
        case 1: return 88200;
        case 2: return 64000;
        case 3: return 48000;
        case 4: return 44100;
        case 5: return 32000;
        case 6: return 24000;
        case 7: return 22050;
        case 8: return 16000;
        case 9: return 12000;
        case 10: return 11025;
        case 11: return 8000;
        case 12: return 7350;
        default:    return 0;
        }
    }
public:
    struct AACS
    {
        uint32_t offset;
        uint32_t len;
        double duration; // ms
    };

    std::vector<AACS> operator()(uint8_t* p, uint32_t size)
    {
        std::vector<AACS> as;
        if (size < ADTS_HEADER_SIZE)
            return as;
        uint32_t cnt = 0;
        uint8_t* tmp = p;
        while(tmp + ADTS_HEADER_SIZE < p+size)
        {
            uint8_t* header = tmp;
            if ((((header[0] << 8) | header[1]) & ADTS_SYNC_MASK) != ADTS_SYNC_PATTERN)
            {   break; }
            AdtsHeader adts_h(tmp);
            if (adts_h.Check() == -1)
            {
                break;
            }
            AACS a;
            a.offset = tmp-p;
            a.len = adts_h.m_FrameLength;
            a.duration = 1000.0/GetSamplingFrequency(adts_h.m_SamplingFrequencyIndex)*1024;
            as.push_back(a);
            tmp += adts_h.m_FrameLength;
            ++cnt;
        }
        return as;
    }
};
bool includeIDR(uint8_t* buf, uint32_t size, bool h264 = false)
{
    if (size < 4)
        return false;
    static int const BLA_W_LP = 16;
    static int const RSV_IRAP_VCL23 = 23;
    static int const SLICE_IDR_PICTURE = 5;
    for (uint32_t i = 2; i < size ; ++i)
    {
        if (0x01 == buf[i] && 0x00 == buf[i-1] && 0x00 == buf[i-2])
        {
            if (buf+i == buf+size-1) // end
                return false;
            if (false == h264)
            {
                uint8_t nal_type = buf[i+1] >> 1 & 0x3F;
                if (nal_type >= BLA_W_LP && nal_type <= RSV_IRAP_VCL23)
                {
                    return true;
                }
            }
            else
            {
                uint8_t nal_type = buf[i+1] & 0x1f;
                if(nal_type == SLICE_IDR_PICTURE)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

using namespace wzd;
CTsDemuxer::CTsDemuxer()
    :m_tag(NULL)
    ,m_pinsOut(NULL)
    ,m_countPin(0)
    ,m_status(S_Stop)
    ,m_isEOF(false)
    ,m_start(MEDIA_FRAME_NONE_TIMESTAMP)
    ,m_clock(0)
    ,_isM3u8(false)
    ,_isNewTSSeg(false)
    ,_hasV(false)
    ,_firstDTSofSEG(MEDIA_FRAME_NONE_TIMESTAMP)
    ,_lastDTSofSEG(MEDIA_FRAME_NONE_TIMESTAMP)
{
    //ctor
}

bool CTsDemuxer::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_pinIn.Create(CLSID_CInputPin,(IFilter*)this),false);

    dom_ptr<IEpoll> spEpoll;
    JCHK(spEpoll.QueryFrom(g_pSite->GetObj()),false);
    JCHK(S_OK == spEpoll->CreatePoint(this,&m_epoll),false);

    _demuxer = wzd::CreateTSDemuxer();
    _cnt = 10240;
    _bufLen = 0;
    _init = false;
    return true;
}

bool CTsDemuxer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CTsDemuxer)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ICallback)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CTsDemuxer::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CTsDemuxer::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CTsDemuxer::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CTsDemuxer::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CTsDemuxer::GetInputPinCount()
{
    return 1;
}

STDMETHODIMP_(IInputPin*) CTsDemuxer::GetInputPin(uint32_t index)
{
    return 0 == index ? m_pinIn : NULL;
}

STDMETHODIMP_(uint32_t) CTsDemuxer::GetOutputPinCount()
{
    return m_countPin;
}

STDMETHODIMP_(IOutputPin*) CTsDemuxer::GetOutputPin(uint32_t index)
{
    return index < m_countPin ? m_pinsOut[index] : NULL;
}

STDMETHODIMP_(IInputPin*) CTsDemuxer::CreateInputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP_(IOutputPin*) CTsDemuxer::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CTsDemuxer::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < S_NB)
    {
        if(cmd != m_status)
        {
            if(S_Stop == m_status)
            {
                JIF(Open());
            }
            else if(S_Stop == cmd)
            {
                JIF(Close());
            }
            if(S_Play == cmd)
            {
                m_pinIn->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
                if(true == _isM3u8)
                {
                    for(uint32_t i=0 ; i<m_countPin ; ++i)
                    {
                        m_pinsOut[i]->SetClock(true);
                    }
                }
            }
            else if(S_Play == m_status)
            {
                if(true == _isM3u8)
                {
                    for(uint32_t i=0 ; i<m_countPin ; ++i)
                    {
                        m_pinsOut[i]->SetClock(false);
                    }
                }
            }
            m_status = (Status)cmd;
        }
        else
            return hr;
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CTsDemuxer::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CTsDemuxer::SetTag(void* pTag)
{
    m_tag = pTag;
}

STDMETHODIMP_(void*) CTsDemuxer::GetTag()
{
    return m_tag;
}

STDMETHODIMP_(double) CTsDemuxer::GetExpend()
{
    return 0;
}

STDMETHODIMP CTsDemuxer::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{

    JCHK(pPin == m_pinIn.p,E_FAIL);
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        if(MMT_DATA != pMT->GetMajor())
            return E_INVALIDARG;

        _session = pPinOut->GetFilter();
        _m3u8_url = "http://";
        _m3u8_url += _session->GetName();
        size_t dot = _m3u8_url.find_last_of('.');
        if(dot != string::npos)
        {
            string format = _m3u8_url.substr(++dot);
            _isM3u8 = format == M3U8_FORMAT_NAME;
            if (_isM3u8)
                _isNewTSSeg = true;
        }
        return S_OK;
    }
    else
    {
        if(NULL != m_pinsOut)
        {
            delete[] m_pinsOut;
            m_pinsOut = NULL;
        }
        m_countPin = 0;
    }
    return hr;
}

STDMETHODIMP CTsDemuxer::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{

    return NULL == pMT || COMPARE_SAME_VALUE == pPin->GetMediaType()->Compare(pMT) ? S_OK : E_INVALIDARG;
}


FILE* fpv = fopen("h264.es", "wb");
FILE* fpa = fopen("aac.es", "wb");
STDMETHODIMP CTsDemuxer::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    HRESULT hr = S_OK;
    JCHK(m_pinIn == pPin,E_INVALIDARG);
    JCHK(NULL != pFrame,E_INVALIDARG);
#if 0
    const IMediaFrame::buf* buf = pFrame->GetBuf();
    std::cout << "frame size is " << buf->size <<std::endl;
    fwrite(buf->data, 1, buf->size, fp);
#endif

    const IMediaFrame::buf* b = pFrame->GetBuf();
    unsigned char* pF = (unsigned char*)b->data;
    unsigned int pFS = b->size;
    while(1)
    {
        // combine 188 buf
        if (pFS > 188-_bufLen)
        {
            memcpy(_buf+_bufLen, pF, 188-_bufLen);
            pF += 188-_bufLen;
            pFS -= 188-_bufLen;
            _bufLen = 0;
        }
        else
        {
            memcpy(_buf+_bufLen, pF, pFS);
            _bufLen = pFS;
            return S_OK;
        }

        // init
        const unsigned char* p188 = _buf;
        if (false ==_init)
        {
            int iret = -1;
            iret = _demuxer->Init(p188);

            if (0 == iret)
            {
                vector<ProgramMapSection> pmt = _demuxer->GetPmt();
                assert(pmt[0].esInfo.size() > 0);
                m_countPin = pmt[0].esInfo.size();
                JCHK(m_pinsOut = new dom_ptr<IOutputPin>[m_countPin],E_OUTOFMEMORY);
                for(uint32_t i=0 ; i<m_countPin ; ++i)
                {
                    dom_ptr<IMediaType> spMT;
                    JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
                    JCHK(m_pinsOut[i].Create(CLSID_COutputPin,(IFilter*)this,false,&i),E_FAIL);
                    spMT->SetSub(stream_type2MediaSubType(pmt[0].esInfo[i].stream_type));
                    m_pinsOut[i]->SetID(pmt[0].esInfo[i].elementary_PID);
                    JIF(m_pinsOut[i]->SetMediaType(spMT));
                    _id_pins.insert(make_pair(m_pinsOut[i]->GetID(), m_pinsOut[i]));
                }
                _init = true;
            }
            else if (0 == --_cnt)
            {
                LOG(0, "find pat pmt failed");
                return E_FAIL;
            }
        }
        else // proc frame
        {
            // read package
            wzd::Packet pkt;

            int iret = -1;
            iret = _demuxer->GetESPacket(pkt,p188);
            if (!iret)
            {
                // create Frame
                dom_ptr<IStream> spStream;
                dom_ptr<IMediaFrame> spFrame;
                spFrame.Create(CLSID_CMediaFrame);
                spFrame.Query(&spStream);
                spStream->Write(pkt.pbuf, pkt.len);
                spFrame->info.dts = pkt.dts;
                spFrame->info.pts = pkt.pts;

                // find pin
                dom_ptr<IOutputPin> pin = _id_pins.find(pkt.pid)->second;
                // query mediatype
                IMediaType* mt = pin->GetMediaType();
                MediaSubType tp = mt->GetSub();
                // parse
                switch(tp)
                {
                case MST_H264:
                    if (includeIDR(pkt.pbuf, pkt.len, true))
                        {
                            spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
                                                    if (_isNewTSSeg)
                            spFrame->info.flag |= MEDIA_FRAME_FLAG_SEGMENT;
                        }
                    //fwrite(pkt.pbuf, 1, pkt.len, fpv);
                    break;
                case MST_H265:
                    if (includeIDR(pkt.pbuf, pkt.len))
                        {
                            spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
                                                    if (_isNewTSSeg)
                            spFrame->info.flag |= MEDIA_FRAME_FLAG_SEGMENT;
                        }
                    break;
                case MST_AAC:
                case MST_MP3:
                    spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
                                            if (_isNewTSSeg && (_hasV == false))
                            spFrame->info.flag |= MEDIA_FRAME_FLAG_SEGMENT;
                    //fwrite(pkt.pbuf, 1, pkt.len, fpa);
                    break;
                default:
                    return E_FAIL;
                }
                JIF(pin->Write(spFrame));
            }
        }
    }
    return S_OK;
}

//IEventCallback
// non-chunked mode

STDMETHODIMP CTsDemuxer::OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
{
    if(ET_Epoll_Timer == type)
    {
        dom_ptr<ILoad> spLoad;
        JCHK(spLoad.QueryFrom(_session),E_FAIL);
        return spLoad->Load(_m3u8_url.c_str(),FT_Source);
    }
    else if(ET_Stream_Read == type)
    {
        HRESULT hr;
        if (_isM3u8)
        {
            switch(_status)
            {
            case RECV_M3u8:
            {

                IStream* pStream = (IStream*)source;
                string m3u8;
                JCHK(0 < param1,E_INVALIDARG);

                m3u8.resize(param1,0);
                JIF(pStream->Read((void*)m3u8.data(),param1));
                LOG(0,"m3u8:%s",m3u8.c_str());
                M3u8Parser::M3u8 m3u = _m3u8Parser->parse(m3u8);
                if (m3u.itsms.size() == 0)
                {
                    LOG(0, "invalid m3u8 file");
                    return E_FAIL;
                }

                 M3u8Parser::M3u8Item* pTsItm = NULL;
                if(_curTsSeg.empty())
                {
                    // get first seg
                    _curTsSeg = m3u.itsms.back().tsLoc;
                    pTsItm = &m3u.itsms.back();
                }
                else
                {
                    for(int i = 0; i < m3u.itsms.size();++i)
                    {
                        if (_curTsSeg == m3u.itsms[i].tsLoc)
                        {
                            if(i < m3u.itsms.size()-1)
                                pTsItm = &m3u.itsms[i+1];
                            break;
                        }
                        else if(i == m3u.itsms.size()-1)
                            pTsItm = &m3u.itsms[i];
                    }
                }

                string name;
                uint32_t wait;
                if(pTsItm != NULL)
                {
                    string ts = pTsItm->tsLoc;
                    if(string::npos != ts.find("://"))
                    {
                        name = ts;
                    }
                    else
                    {
                        name = "http://";
                        name += _session->GetName();
                        size_t slash = name.find_last_of('/');
                        JCHK(string::npos != slash,E_INVALIDARG);
                        if('/' == ts.front())
                            name.replace(slash,string::npos,ts);
                        else
                            name.replace(++slash,string::npos,ts);
                    }
                    m_duration = (uint32_t)(pTsItm->dur * 1000);
                    wait = 0;
                    _status = RECV_TS;
                    _isNewTSSeg = true;//每收到一次M3u8表明一个新片要开始了。

                    LOG(0,"m3u8:[%s] parse finsh will load ts:[%s]",_session->GetName(),name.c_str());

                    dom_ptr<ILoad> spLoad;
                    JCHK(spLoad.QueryFrom(_session),E_FAIL);
                    JIF(spLoad->Load(name.c_str(),FT_Source));
                }
                else
                {
                    JIF(m_epoll->SetTimer(0,1000,true));
                }
                return E_HANGUP;
            }
            break;
            case RECV_TS:
            {
                IStream* pStream = (IStream*)source;
                uint64_t prv = pStream->GetStatus().read_total_size;
                if (false == _init)
                {
                    // init demuxer
                    hr = initDemuxer(pStream);
                    uint64_t cur = pStream->GetStatus().read_total_size;
                    int left = int(cur - prv);
                    JCHK(left <= param1,E_FAIL);

                    if (left == param1)
                    {
                        _status = RECV_M3u8;

                        JIF(m_epoll->SetTimer(0,m_duration,true));

                        hr = E_HANGUP;
                    }
                }
                if (_init)
                {
                    hr = getPacket(pStream);
                    uint64_t cur = pStream->GetStatus().read_total_size;
                    int left = int(cur - prv);

                    JCHK(left <= param1,E_FAIL);

                    if (left == param1)
                    {
                        _status = RECV_M3u8;

                        uint64_t download = m_epoll->GetClock() - m_clock;
                        int64_t duration = int64_t((_lastDTSofSEG - m_start)/10000 - download - 2000);

                        if(duration > 0)
                        {
                            LOG(0,"next ts clip wait %ld ms",duration);
                            JIF(m_epoll->SetTimer(0,(uint32_t)duration,true));
                        }
                        else
                        {
                            dom_ptr<ILoad> spLoad;
                            JCHK(spLoad.QueryFrom(_session),E_FAIL);
                            JIF(spLoad->Load(_m3u8_url.c_str(),FT_Source));
                        }
                        hr = E_HANGUP;
                    }
                }
                return hr;
            }
            break;
            }

        }
        IStream* pStream = (IStream*)source;
        if (false == _init)
        {
            // init demuxer
            JIF(initDemuxer(pStream));
        }
        else
        {
            JIF(getPacket(pStream));
        }
        return hr;
#if 0
        char buf[1024];
        while(E_AGAIN != pStream->Read(buf, sizeof(buf)/sizeof(char)))
        {
            fwrite(buf, 1, sizeof(buf), fp);
        }
        return E_AGAIN;
#endif
    }
    return m_ep->Notify(type,param1,param2,param3);
}


HRESULT CTsDemuxer::Open()
{
    HRESULT hr = S_OK;
    _m3u8Parser =new M3u8Parser;
    m_start = MEDIA_FRAME_NONE_TIMESTAMP;
    m_clock = 0;
    return hr;
}

HRESULT CTsDemuxer::Close()
{
    HRESULT hr = S_OK;
    delete _m3u8Parser;
    return hr;
}

HRESULT CTsDemuxer::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL != pMtIn)
    {
        if(MMT_DATA == pMtIn->GetMajor())
        {
            if(MST_NONE == pMtIn->GetSub() || MST_MPEG2TS == pMtIn->GetSub() || MST_HLS == pMtIn->GetSub())
                return 1;
        }
    }
    return E_INVALIDARG;
}

HRESULT CTsDemuxer::initDemuxer(IStream* strm)
{
    HRESULT hr = S_OK;
    int iret = -1;
    char buf[300];

    do
    {
        const unsigned char* pBuf = (unsigned char* )buf;
        JIF(strm->Read(buf, 188));
        iret = _demuxer->Init(pBuf);
        --_cnt;
    }
    while(iret && _cnt > 0);
    if (0 == iret)
    {
        vector<ProgramMapSection> pmt = _demuxer->GetPmt();
        assert(pmt.size() > 0);
        m_countPin = pmt[0].esInfo.size();
        JCHK(m_pinsOut = new dom_ptr<IOutputPin>[m_countPin],E_OUTOFMEMORY);
        for(uint32_t i=0 ; i<m_countPin ; ++i)
        {
            dom_ptr<IMediaType> spMT;
            JCHK(spMT.Create(CLSID_CMediaType),E_FAIL);
if (pmt[0].esInfo[i].stream_type == 0x1b
    || 0x24 == pmt[0].esInfo[i].stream_type)
{
    _hasV = true;
}
            spMT->SetSub(stream_type2MediaSubType(pmt[0].esInfo[i].stream_type));
            JCHK(m_pinsOut[i].Create(CLSID_COutputPin,(IFilter*)this,false,&i),E_FAIL);
            m_pinsOut[i]->SetID(pmt[0].esInfo[i].elementary_PID);
            JIF(m_pinsOut[i]->SetMediaType(spMT));
            _id_pins.insert(make_pair(m_pinsOut[i]->GetID(), m_pinsOut[i]));
        }
        _init = true;
        cout << "init complete!" << endl;

        JIF(m_ep->Notify(ET_Filter_Build,0,(IFilter*)this));
        if(true == _isM3u8)
        {
            for(uint32_t i=0 ; i<m_countPin ; ++i)
            {
                //m_pinsOut[i]->SetThrottle(true);
            }
        }
        return hr;
    }
    LOG(0, "not found pat pmt");
    return E_FAIL;
}

HRESULT CTsDemuxer::getPacket(IStream* strm)
{
    HRESULT hr = S_OK;
    unsigned char buf[300];
    const unsigned char* pBuf = (unsigned char*)buf;
    wzd::Packet pkt;

    while(true)
    {
        JIF(strm->Read(buf, 188));
        int iret = -1;
        iret = _demuxer->GetESPacket(pkt,pBuf);
        if (!iret)
        {
            //cout << pkt.pts << '\t'<<pkt.len <<endl;
            // create Frame
            dom_ptr<IStream> spStream;
            dom_ptr<IMediaFrame> spFrame;
            spFrame.Create(CLSID_CMediaFrame);
            spFrame.Query(&spStream);
            spStream->Write(pkt.pbuf, pkt.len);
            spFrame->info.dts = (double)pkt.dts*10000000/90000+0.5;
            spFrame->info.pts = (double)pkt.pts*10000000/90000+0.5;
//            cout << "ts pts: " << spFrame->info.pts << endl;
//            cout << "ts dts: " << spFrame->info.dts << endl;

            // find pin
            dom_ptr<IOutputPin> pin = _id_pins.find(pkt.pid)->second;
            // query mediatype
            IMediaType* mt = pin->GetMediaType();
            MediaSubType tp = mt->GetSub();
            // parse
            switch(tp)
            {
            case MST_H264:
                if (includeIDR(pkt.pbuf, pkt.len, true))
                    {
                        spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
                        if (_isNewTSSeg)
                            {
                                spFrame->info.flag |= MEDIA_FRAME_FLAG_SEGMENT;
                                _firstDTSofSEG = spFrame->info.dts;
                            }
                    }
                    _lastDTSofSEG = spFrame->info.dts;
                //fwrite(pkt.pbuf, 1, pkt.len, fpv);
                break;
            case MST_H265:
                if (includeIDR(pkt.pbuf, pkt.len))
                    {
                        spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
                        if (_isNewTSSeg)
                            {
                                spFrame->info.flag |= MEDIA_FRAME_FLAG_SEGMENT;
                                _firstDTSofSEG = spFrame->info.dts;
                            }
                    }
                    _lastDTSofSEG = spFrame->info.dts;
                break;
            case MST_AAC:
                {

                AACFrameSpliter spliter;
                std::vector<AACFrameSpliter::AACS> frms = spliter(pkt.pbuf,pkt.len);
            for(int i = 0; i < frms.size(); ++i)
            {
            dom_ptr<IStream> spStream;
            dom_ptr<IMediaFrame> spFrame;
            spFrame.Create(CLSID_CMediaFrame);
            spFrame.Query(&spStream);
            spStream->Write(pkt.pbuf+frms[i].offset, frms[i].len);
            spFrame->info.dts = (double)pkt.dts*10000000/90000+0.5 + frms[i].duration*10000*i;
            spFrame->info.pts = (double)pkt.pts*10000000/90000+0.5 + frms[i].duration*10000*i;
            spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;

            if (false ==_hasV)
                {
                    _lastDTSofSEG = spFrame->info.dts;
                    if (_isNewTSSeg)
                    {
            spFrame->info.flag |= MEDIA_FRAME_FLAG_SEGMENT;
                    _firstDTSofSEG = spFrame->info.dts;
                    }
                }
                //fwrite(pkt.pbuf, 1, pkt.len, fpa);
            JIF(pin->Write(spFrame));

            }
            _demuxer->RecyclePacket(pkt);
            return S_OK;
                }
                break;
                case MST_MP3:
            default:
                return E_FAIL;
            }


            if(MEDIA_FRAME_NONE_TIMESTAMP == m_start || _firstDTSofSEG < m_start || _lastDTSofSEG < _firstDTSofSEG)
            {
                if(_lastDTSofSEG < _firstDTSofSEG)
                    _firstDTSofSEG = _lastDTSofSEG;

                m_start = _firstDTSofSEG;
                m_clock = m_epoll->GetClock();
            }
            JIF(pin->Write(spFrame));
            _demuxer->RecyclePacket(pkt);
        }

    }
    return hr;
}
MediaSubType CTsDemuxer::stream_type2MediaSubType(int t)
{
    switch(t)
    {
    case 0x1b:
        return MST_H264;
    case 0xf:
        return MST_AAC;
    case 0x24:
        return MST_H265;
    default:
        return MST_NONE;
    }
}



// include cpp fils
#include "./tsdemuxer/base.cpp"
#include "./tsdemuxer/Bits.cpp"
#include "./tsdemuxer/CElementStreamPacker.cpp"
#include "./tsdemuxer/CElementStreamParser.cpp"
#include "./tsdemuxer/CProgramSpecialInformationParser.cpp"
#include "./tsdemuxer/CTransportStreamParser.cpp"
#include "./tsdemuxer/CTSDeMuxer.cpp"
#include "./tsdemuxer/ITSDemuxer.cpp"
#include "./tsdemuxer/M2TSDemuxer.cpp"
