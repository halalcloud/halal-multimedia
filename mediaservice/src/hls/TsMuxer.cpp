#include "TsMuxer.h"
#include <iostream>
#include "../src/Url.cpp"
CTsMuxer::CTsMuxer()
    :m_status(S_Stop)
    ,m_isOpen(NULL)
    ,m_isFirst(false)
    ,m_is_live(false)
    ,m_pTag(NULL)
    ,m_index(0)
    ,m_master(0)
    ,_hasA(false)
    ,_hasV(false)
    ,_frm_flags(nullptr)
    ,_ff(nullptr)
    ,_fmtA(ADTS)
    ,_fmtV(ANNEXB)
    ,_flash(true)
    ,_genM3u8(false)
    ,_asc2adts(NULL)
    ,_segCnt(0)
{
    //ctor
}


bool CTsMuxer::FinalConstruct(Interface* pOuter,void* pParam)
{
    dom_ptr<IMediaType> spMT;
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this),false);
    JCHK(spMT.Create(CLSID_CMediaType),false);
    JCHK(S_OK == spMT->SetSub(MST_MPEG2TS),false);
    JCHK(m_pinOut.Create(CLSID_COutputPin,(IFilter*)this),false);
    JCHK(S_OK == m_pinOut->SetMediaType(spMT),false);

    return true;
}

bool CTsMuxer::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CTsMuxer)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CTsMuxer::GetType()
{
    return FT_Transform;
}

STDMETHODIMP CTsMuxer::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CTsMuxer::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CTsMuxer::GetFlag()
{
    return 0;
}

STDMETHODIMP_(uint32_t) CTsMuxer::GetInputPinCount()
{
    return m_inputs.size();
}

STDMETHODIMP_(IInputPin*) CTsMuxer::GetInputPin(uint32_t index)
{
    return index < m_inputs.size() ? m_inputs.at(index) : NULL;
}

STDMETHODIMP_(uint32_t) CTsMuxer::GetOutputPinCount()
{
    return 1;
}

STDMETHODIMP_(IOutputPin*) CTsMuxer::GetOutputPin(uint32_t index)
{
    return 0 == index ? m_pinOut : NULL;
}

STDMETHODIMP_(IInputPin*) CTsMuxer::CreateInputPin(IMediaType* pMT)
{
    JCHK(false == m_isOpen,NULL);
    JCHK(NULL != m_pinOut->GetConnection(),NULL);

    MediaSubType sub = pMT->GetSub();
    if(MST_NONE == sub)
        return NULL;
    if(MMT_DATA == pMT->GetMajor())
    {
        if(MST_GOSUN_DATA != sub)
            return NULL;
        if(true == m_is_live)
            return NULL;
    }

    dom_ptr<IInputPin> spPin;
    size_t index = m_inputs.size();
    JCHK(spPin.Create(CLSID_CInputPin,this,false,&index),NULL);
    JCHK(S_OK == spPin->SetMediaType(pMT),NULL);
    m_inputs.push_back(spPin);
    return spPin;
}

STDMETHODIMP_(IOutputPin*) CTsMuxer::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CTsMuxer::Notify(uint32_t cmd)
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
                if(m_slices != NULL)
                {
                    hr = S_FALSE;
                }
                else
                {
                    for(uint32_t i=0 ; i<m_inputs.size() ; ++i)
                    {
                        dom_ptr<IInputPin>& pin = m_inputs.at(i);
                        pin->SetFlag(MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_NEWSEGMENT);
                    }
                    m_isFirst = true;
                }
            }
            m_status = (Status)cmd;
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CTsMuxer::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CTsMuxer::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CTsMuxer::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CTsMuxer::GetExpend()
{
    return 0.0;
}

STDMETHODIMP CTsMuxer::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    JCHK(NULL != pPin,E_INVALIDARG);

    HRESULT hr = S_OK;

    if(NULL != pMT)
    {
        JCHK(NULL != m_pinOut->GetConnection(),E_FAIL);
        JCHK(MST_NONE != pMT->GetSub(),E_INVALIDARG);
        if(MMT_VIDEO == pMT->GetMajor())
        {
            //        int width,height;
            //        int64_t duration;
            //        JIF(pMT->GetVideoInfo(NULL,&width,&height,NULL,NULL,&duration));
        }
        else if(MMT_AUDIO == pMT->GetMajor())
        {
            //        int channels,sample_rate,frame_size;
            //        JIF(pMT->GetAudioInfo(NULL,NULL,&channels,&sample_rate,&frame_size));
        }
        else if(MMT_DATA == pMT->GetMajor())
        {
            JCHK(MST_GOSUN_DATA == pMT->GetSub(),E_INVALIDARG);
            JCHK(false == m_is_live,E_FAIL);
            if(0 < _segCnt)
            {
                dom_ptr<IProfile> spProfile;
                JCHK(spProfile.QueryFrom(pMT),E_FAIL);
                JCHK(spProfile->Write("buffer", _segCnt),E_FAIL);
            }
        }
    }
    return hr;
}

STDMETHODIMP CTsMuxer::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    if(NULL != pMT)
    {
        if(MST_NONE == pMT->GetSub())
            return pMT->SetSub(MST_MPEG2TS);
        else
        {
            if(MST_HLS == pMT->GetSub())
            {
                if(GUID_CMP(pPinIn->GetFilter()->Class().clsid,CLSID_CPublishRender))
                    m_is_live = true;
                else
                {
                    dom_ptr<IProfile> spProfile;
                    JCHK(spProfile.QueryFrom(pMT),E_FAIL);
                    spProfile->Read("buffer", _segCnt);
                }
                return S_OK;
            }
            else if(MST_MPEG2TS == pMT->GetSub())
            {
                m_is_live = true;
                return S_OK;
            }
            else
                return E_INVALIDARG;
        }
    }
    else
        return S_OK;
}

STDMETHODIMP CTsMuxer::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    JCHK(NULL != pFrame,E_INVALIDARG);

    HRESULT hr = S_OK;

    CLocker locker(m_locker);
    //printf("CTsMuxer::OnWriteFrame pPin:%p pFrame:%ld flag:%u\n",pPin,pFrame->info.dts,pFrame->info.flag);
    if(NULL != pFrame->GetBuf())
    {
        // get type
        IMediaType* mt = pPin->GetMediaType();
        MediaMajorType mmt = mt->GetMajor();

        if(0 != (pFrame->info.flag&MEDIA_FRAME_FLAG_SEGMENT))
        {
            //printf("CCTsMuxer::OnWriteFrame %s stream dts:%ld flag:%d\n",mt->GetMajorName(),pFrame->info.dts,pFrame->info.flag);
        }
        //LOG(0,"ts muxer input [%s:%s] frame dts:%ld pts:%ld flag:%d",mt->GetMajorName(),mt->GetSubName(),pFrame->info.dts,pFrame->info.pts,pFrame->info.flag);

        // recalc timestamp
        wzd::calc_timestamp::AVRational in, out;
        out.num = 1;
        out.den = 90000; // 90K hz
        in.num = 1;
        in.den = 10000000; // 100 ns
        wzd::calc_timestamp c(in, out);

        std::unique_ptr<wzd::IMediaFrame> f(new wzd::IMediaFrame);
        // init frame struct
        f->info.dts = c.calc(pFrame->info.dts);
        f->info.pts = c.calc(pFrame->info.pts);

        uint8_t* Data = nullptr;
        unique_ptr<uint8_t[]> _auto_buf;
        size_t Size = 0;
        const IMediaFrame::buf* b = pFrame->GetBuf();

        unique_ptr<uint8_t,wzd::h264_mp4toannexb::buf_deleter> temp(NULL,wzd::h264_mp4toannexb::buf_deleter(_mp4To));
        if (mmt == MMT_VIDEO)
        {
            f->strmID = 0;
            //                std::cout << "video: pts " << pFrame->info.pts << "dts: "<< pFrame->info.dts << "\tff: " << _ff<< std::endl;
            if (_fmtV == MP4)
            {
                _mp4To->h264_mp4toannexb_filter(&Data, (int*)&Size, (uint8_t*)b->data, (int)b->size);
                JCHK(NULL != Data,E_INVALIDARG);
                temp.reset(Data);
            }
            else
            {
                Data = b->data;
                Size = b->size;
            }
        }
        else if (mmt == MMT_AUDIO)
        {
            f->strmID = 10;
            if (_fmtA == ASC)
            {
                _asc2adts->aac_asc2adts_filter((uint8_t*)b->data, (size_t)b->size, &Data, &Size);
                JCHK(NULL != Data,E_INVALIDARG);
                _auto_buf.reset(Data);
            }
            else
            {
                Data = b->data;
                Size = b->size;
            }
        }
        else
        {
            return E_FAIL;
        }

        f->lpdata = Data;
        f->dwSize  = Size;


        if (_genM3u8)
        {

            if (0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_SEGMENT))
            {
                if (_ff == nullptr) // save first timestamp
                {
                         _ff = pFrame;
                }
                else// need recreate frame
                {
                        _frm_flags = pFrame;
                        //printf("waite flash dts:%ld flag:%d\n",pFrame->info.dts,pFrame->info.flag);
                        _flash = true;
                }
            }
            if (_ff == nullptr)
            {
                return S_OK;
            }

            _mux->OnWriteFrame(f.get(), _flash);
            _mux->flush();

        }
        else // ts
        {
            bool forcePMT = false;
            // set pid
            if (mmt == MMT_VIDEO)
            {
                f->strmID = 0;
                if (0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT))
                    forcePMT = true;
                //            std::cout << "video: pts " << pFrame->nfo.pts << "dts: "<< pFrame->info.dts << std::endl;
                //            std::cout << "90kvideo: pts " << f->info.pts << "dts: "<< f->info.dts << std::endl;
            }

            _mux->OnWriteFrame(f.get(),forcePMT);

            //save info to member
            if (_hasV && mmt == MMT_VIDEO) // 有视频且碰到视频帧
            {
                _frm_flags = pFrame;
                // call flash
                _mux->flush();
            }
            else if (_hasV == false)//无视频，仅音频。帧帧输出
            {
                _frm_flags = pFrame;
                _mux->flush();
            }
        }
    }
    if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_EOF))
    {
        pFrame->Clear();
        for(uint32_t i=0 ; i<m_inputs.size() ; ++i)
        {
            dom_ptr<IInputPin>& pin = m_inputs.at(i);
            if(false == pin->IsEnd())
                return S_FALSE;
        }
        _mux->flush();
        JIF(m_pinOut->Write(pFrame));
    }
    return hr;
}

//FILE* g_fp = nullptr;
HRESULT CTsMuxer::Open()
{
    HRESULT hr = S_OK;

    if(true == m_isOpen)
        return hr;

    JCHK(NULL != m_pinOut->GetConnection(),E_FAIL);
    //g_fp = fopen("tsts.ts", "wb");
    string str_m3u8;
    IProfile::val* pVal;
    IMediaType* pMT;
    dom_ptr<IProfile> spProfile;
    JCHK(pMT = m_inputs.front()->GetMediaType(),E_FAIL);
    m_index = 0;
    if(MST_GOSUN_DATA == pMT->GetSub())
    {
        JCHK(spProfile.QueryFrom(pMT),E_FAIL);
        JCHK(pVal = spProfile->Read("slices"),E_FAIL);
        spProfile = (IProfile*)pVal->value;

        uint16_t segCnt = (uint16_t)spProfile ->Count();
        JCHK(3 < segCnt,E_FAIL);

        _m3.reset(new wzd::M3u8Generator(segCnt-1));

        char* pEnd;
        string item;
        int64_t pre = 0;
        IProfile::Name key_name = NULL;
        double dur = 0.0;
        while(NULL != (key_name = spProfile ->Next(key_name)))
        {
            JCHK(pVal = spProfile ->Read(key_name),E_INVALIDARG);
            string str = (const char*)pVal->value;
            if(false == str.empty())
            {
                size_t dot = str.find_last_of('.');
                if(dot != string::npos)
                    str.replace(dot,string::npos,".ts");
                else
                    str += ".ts";

                size_t slash = str.find_last_of('/');
                JCHK(slash != string::npos,E_INVALIDARG);
                string time = str.substr(++slash);

                pEnd = NULL;
                uint32_t key = (uint32_t)strtoul(key_name,&pEnd,10);
                JCHK(NULL != pEnd,INVALID_FD);

                pEnd = NULL;
                int64_t cur = strtol(time.c_str(),&pEnd,10);
                JCHK(NULL != pEnd,INVALID_FD);

                if(false == item.empty())
                {
                    JCHK2(cur > pre,E_INVALIDARG,"cur:%ld pre:%ld",cur,pre);
                    dur = double(cur - pre)/1000.0;
                    str_m3u8 = _m3->append((char*)item.c_str(), dur,m_index);
                }
                m_index = key;
                item = str;
                pre = cur;
            }
            else
            {
                str_m3u8 += "\n#EXT-X-ENDLIST";
                break;
            }
        }

        JCHK(pMT = m_pinOut->GetMediaType(),E_FAIL);
        dom_ptr<IMediaFrame> spFrame;
        JIF(m_pinOut->AllocFrame(&spFrame));
        spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_EOF;
        JIF(spFrame->SetBuf(0,str_m3u8.size(),str_m3u8.c_str()));
        JIF(pMT->SetFrame(spFrame));
    }
    else
    {
        JCHK(pMT = m_pinOut->GetMediaType(),E_FAIL);
        JCHK(spProfile.QueryFrom(pMT),E_FAIL);

        int is264 = -1;
        int  isaac = -1;

        for(uint32_t i=0 ; i<m_inputs.size() ; ++i)
        {
            dom_ptr<IInputPin>& pin = m_inputs.at(i);
            IMediaType* mt = pin->GetMediaType();
            MediaSubType smt = mt->GetSub();

            if(NULL != pin->GetConnection())
            {
                switch(smt)
                {
                case MST_H264:
                    {

                                    bool global_header;
                    uint8_t* extra_data;
                    int extra_size;
                    JIF(mt->GetStreamInfo(NULL,NULL,NULL,&global_header,&extra_data,&extra_size));

                    if(true == global_header && 4 <= extra_size && NULL != extra_data)
                    {
                        _fmtV = MP4;
                        _mp4To = new wzd::h264_mp4toannexb(extra_data, extra_size);
                    }
                    is264 = 1;
                    _hasV = true;
                    }
                    break;
                case MST_H265:
                    is264 = 2;
                    _hasV = true;
                    break;
                case MST_AAC:
                {
                    bool global_header;
                    uint8_t* extra_data;
                    int extra_size;
                    JIF(mt->GetStreamInfo(NULL,NULL,NULL,&global_header,&extra_data,&extra_size));

                    if(true == global_header && 2 <= extra_size && NULL != extra_data)
                    {
                        _fmtA = ASC;
                        _asc2adts = new wzd::aac_asc2adts(extra_data, extra_size);
                    }
                    isaac = 1;
                    _hasA = true;
                }
                    break;
                case MST_MP3:
                    isaac = 2;
                    _hasA = true;
                    break;
                default:
                    return E_FAIL;
                }
            }
        }
        _mux.reset(new wzd::CTsMuxer( isaac == 2? true:false,is264 ==2?true:false));
        _mux->InternalOpen(this, _hasV, _hasA);
        JIF(m_pinOut->AllocFrame(&_buf_frm));

        MediaSubType smt = pMT->GetSub();
        if (MST_HLS == smt)
            _genM3u8 = true;
        if (_genM3u8)
        {
            dom_ptr<IMediaFrame> spFrame;
            JIF(m_pinOut->AllocFrame(&spFrame));
            spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_EOF;

            uint16_t segCnt = 0 >= _segCnt ? 3 : (uint16_t)_segCnt;
            _m3.reset(new wzd::M3u8Generator(segCnt));

            JCHK(pVal = spProfile->Read("stream_id"),E_FAIL);
            JIF(m_url.SetStreamID(NULL,(const char*)pVal->value));
            str_m3u8 = "#EXTM3u\r\n#EXT-X-TARGETDURATION:0\r\n#EXT-X-ENDLIST";

            JIF(spFrame->SetBuf(0,str_m3u8.size(),str_m3u8.c_str()));
            JIF(pMT->SetFrame(spFrame));
        }
    }
    //JCHK(m_spOutput.Create(CLSID_CFileStream),E_FAIL);
    //JIF(m_spOutput->Open("out.ts",FT_Render));
    m_isOpen = true;
    return hr;
}

HRESULT CTsMuxer::Close()
{
    HRESULT hr = S_OK;

    if(false == m_isOpen)
        return hr;
    if(_mux)
        _mux->InternalClose();
    m_indexs.clear();
    _ff = NULL;
    _frm_flags = NULL;

    if (_asc2adts != NULL)
    {
        delete _asc2adts;
        _asc2adts = NULL;
    }
    if(m_spOutput != NULL)
    {
        m_spOutput->Close();
        m_spOutput = NULL;
    }
    m_isOpen = false;
    return hr;
}

HRESULT CTsMuxer::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    if(NULL != pMtOut)
    {
        if(MST_MPEG2TS == pMtOut->GetSub() || MST_HLS == pMtOut->GetSub())
            return 1;
    }
    return E_INVALIDARG;
}

HRESULT CTsMuxer::write(unsigned char *p, unsigned int size)
{
    HRESULT hr = S_OK;
    if (size == 0) //关闭的时候会刷新。如果缓存已空，则长度为0
        {
//            LOG(0, "end.....");
    return hr;
        }
    //fwrite(p, sizeof(char), size,g_fp);

    if (_genM3u8)
    {
        if (_flash)
        {
             if (_frm_flags == NULL)
             {
              _flash = false; // already write PAT/PMT
             }
             else
             {


            double dur = (_frm_flags->info.dts - _ff->info.dts)/10000000.0;
            _buf_frm->info = _ff->info;
            //LOG(0,"Write frame:%ld",_buf_frm->info.dts);

            // write ts segment to downstream
            //printf("CCTsMuxer::write dts:%ld flag:%d\n",_buf_frm->info.dts,_buf_frm->info.flag);

            m_pinOut->Write(_buf_frm);
            if(m_spOutput != NULL)
                m_spOutput->Write(_buf_frm.p,0,IStream::WRITE_FLAG_FRAME);
            std::ostringstream ostr;
            ostr << _ff->info.dts;
            ostr << ".ts";

            ostr << "?";
            ostr << STREAM_NAME;
            ostr << "=";
            ostr << m_url.GetStreamName();

            if (false == m_url.m_host.empty() && m_url.m_host != HOST_NAME)
            {
                ostr << "&vhost=";
                ostr << m_url.m_host;
            }
            //* for test
            //            std::ostringstream ostr1;
            //            ostr1 << _ff->info.dts;
            //            ostr1 << ".ts";

            //            FILE* fp = nullptr;
            //            fp = fopen(ostr1.str().c_str(), "wb");
            //            // */
            //            uint32_t cnt = MAX_MEDIA_FRAME_BUF_COUNT;
            //            const IMediaFrame::buf* b = _buf_frm->GetBuf(0, &cnt);
            //            for(int i = 0; i < cnt; ++i)
            //            {
            //                fwrite(b->data, 1, b->size, fp);
            //                b++;
            //            }

            //            fclose(fp);
            //            fp = fopen("zw.m3u8", "w");
            string m3u8 = _m3->append(const_cast<char*>(ostr.str().c_str()), dur,++m_index);
            //            fwrite(m3u8.c_str(), 1, m3u8.size(), fp);
            //            fclose(fp);

            // write m3u8 content to profile
            dom_ptr<IMediaFrame> spFrame;
            JIF(m_pinOut->AllocFrame(&spFrame));
            JIF(spFrame->SetBuf(0,m3u8.size(),m3u8.c_str()));
            spFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT|MEDIA_FRAME_FLAG_EOF;
            JIF(m_pinOut->GetMediaType()->SetFrame(spFrame));

            _buf_frm = NULL;
            JIF(m_pinOut->AllocFrame(&_buf_frm));
            _flash = false;
            _ff = _frm_flags;
             }
        }
        dom_ptr<IStream> spStream;
        _buf_frm.Query(&spStream);
        JIF(spStream->Write(p, size));
    }
    else
    {

        dom_ptr<IStream> spStream;
        _buf_frm.Query(&spStream);
        JIF(spStream->Write(p, size));

        if (_frm_flags != nullptr)
        {
            _buf_frm->info = _frm_flags->info;
            m_pinOut->Write(_buf_frm);
            _buf_frm = NULL;
            JIF(m_pinOut->AllocFrame(&_buf_frm));
            _frm_flags = nullptr;
        }
    }
    return hr;
}

#include <memory>
#include <cassert>
#include <string.h>
#include <limits>
#include <stdint.h>
#include <stdlib.h>
#define INT_MAX 0x7FFFFFFF
namespace wzd
{
// muxer files
#include "tsmuxer/TsMuxer.cpp"
#include "tsmuxer/MemoryCache.cpp"
#include "tsmuxer/BitWriter.cpp"

aac_asc2adts::aac_asc2adts(uint8_t *buf, uint32_t size)
    :_extra(NULL)
{
    if(NULL == buf || 0 == size)
        throw std::runtime_error("invalid arg");
    _extra.reset(new uint8_t[size]);
    _extr_size = size;
    GetBitContext gb;
    PutBitContext pb;
    MPEG4AudioConfig m4ac;
    memset(&m4ac,0,sizeof(m4ac));
    int off;

    init_get_bits(&gb, buf, size * 8);
    if(0 <= (off = avpriv_mpeg4audio_get_config(&m4ac, buf, size * 8, 1)))
        ;//,E_INVALIDARG);

    skip_bits_long(&gb, off);


    memset(&_adts_cxt,0,sizeof(ADTSContext));
    _adts_cxt.objecttype        = m4ac.object_type - 1;
    _adts_cxt.sample_rate_index = m4ac.sampling_index;
    _adts_cxt.channel_conf      = m4ac.chan_config;

    if (3U < _adts_cxt.objecttype)
        throw std::runtime_error("MPEG-4 AOT is not allowed in ADTS");//,_adts_cxt.objecttype+1;
    if (15 == _adts_cxt.sample_rate_index)
        throw std::runtime_error("Escape sample rate index illegal in ADTS");
    if (0 != get_bits(&gb, 1))
        throw std::runtime_error("960/120 MDCT window is not allowed in ADTS");
    if (0 != get_bits(&gb, 1))
        throw std::runtime_error("Scalable configurations are not allowed in ADTS");
    if (0 != get_bits(&gb, 1))
        throw std::runtime_error("Extension flag is not allowed in ADTS");

    if (!_adts_cxt.channel_conf) {
        init_put_bits(&pb, _adts_cxt.pce_data, MAX_PCE_SIZE);

        put_bits(&pb, 3, 5); //ID_PCE
        _adts_cxt.pce_size = (avpriv_copy_pce_data(&pb, &gb) + 3) / 8;
        flush_put_bits(&pb);
    }

    _adts_cxt.write_adts = 1;
}

size_t aac_asc2adts::aac_asc2adts_filter(uint8_t *in, size_t insize, uint8_t **buf, size_t *size)
{
    *size = (unsigned)ADTS_HEADER_SIZE + _adts_cxt.pce_size + insize;
    *buf = new uint8_t[*size];
    uint32_t  sz = adts_write_frame_header(*buf,*size);
    memcpy(*buf +sz, in,insize);
    return 0;
}

size_t aac_asc2adts::adts_write_frame_header(uint8_t *buf, size_t size)
{
    PutBitContext pb;
    init_put_bits(&pb, buf, ADTS_HEADER_SIZE);

    /* adts_fixed_header */
    put_bits(&pb, 12, 0xfff);   /* syncword */
    put_bits(&pb, 1, 0);        /* ID */
    put_bits(&pb, 2, 0);        /* layer */
    put_bits(&pb, 1, 1);        /* protection_absent */

    put_bits(&pb, 2, _adts_cxt.objecttype); /* profile_objecttype */
    put_bits(&pb, 4, _adts_cxt.sample_rate_index);
    put_bits(&pb, 1, 0);        /* private_bit */
    put_bits(&pb, 3, _adts_cxt.channel_conf); /* channel_configuration */
    put_bits(&pb, 1, 0);        /* original_copy */
    put_bits(&pb, 1, 0);        /* home */
    /* adts_variable_header */
    put_bits(&pb, 1, 0);        /* copyright_identification_bit */
    put_bits(&pb, 1, 0);        /* copyright_identification_start */
    put_bits(&pb, 13, size); /* aac_frame_length */
    put_bits(&pb, 11, 0x7ff);   /* adts_buffer_fullness */
    put_bits(&pb, 2, 0);        /* number_of_raw_data_blocks_in_frame */
    flush_put_bits(&pb);

    if(0 < _adts_cxt.pce_size)
    {
        memcpy(buf+ADTS_HEADER_SIZE,_adts_cxt.pce_data,_adts_cxt.pce_size);
    }
    return ADTS_HEADER_SIZE + _adts_cxt.pce_size;

}

#define AV_INPUT_BUFFER_PADDING_SIZE 32

#define AV_LOG_QUIET    -8
#define AV_LOG_PANIC     0
#define AV_LOG_FATAL     8
#define AV_LOG_ERROR    16
#define AV_LOG_WARNING  24
#define AV_LOG_INFO     32
#define AV_LOG_VERBOSE  40
#define AV_LOG_DEBUG    48
#define AV_LOG_TRACE    56

#define AVERROR(e) (e)


#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

h264_mp4toannexb::h264_mp4toannexb(unsigned char* extra_data, unsigned int extra_size)
{
    if (extra_size < 6 || !extra_data)
        throw std::runtime_error("invalid arg");
    //_extra_data = new uint8_t[extra_size];
    //memcpy(_extra_data, extra_data, extra_size);
    _extra_data.reset(new uint8_t[extra_size]);
    memcpy(_extra_data.get(), extra_data, extra_size);
    _extr_size = extra_size;
    _cxt._data_size = 1 * 1024 * 1024;
    _cxt._used = 0;
    _cxt._data.reset(new uint8_t[_cxt._data_size]);

}
void h264_mp4toannexb::toStand(const uint8_t* buf, uint32_t size)
{
    if (size > _cxt._data_size)
    {
        _cxt._data_size = size;
        _cxt._data.reset(new uint8_t[_cxt._data_size]);
        LOG(0,"warning: video size greater than 1MB, may be error!");
    }
    const uint8_t* ptmp = buf;
    uint8_t* pData = _cxt._data.get();
    static const int STARTCODE = 4;
    while(ptmp < buf + size)
    {
        // get size
        uint32_t nalSize;
        uint8_t* pNalSize = (uint8_t*)&nalSize;
        pNalSize[3] = ptmp[0];
        pNalSize[2] = ptmp[1];
        pNalSize[1] = ptmp[2];
        pNalSize[0] = ptmp[3];

        // move pointer
        ptmp += sizeof(uint32_t);
        ptmp += STARTCODE;
        nalSize -= STARTCODE;

        // write size
        pData[0] = pNalSize[3];
        pData[1] = pNalSize[2];
        pData[2] = pNalSize[1];
        pData[3] = pNalSize[0];
        pData += 4;
        // nal
        if (ptmp + nalSize > buf +size)
            break;
        memcpy(pData, ptmp, nalSize);
        pData += nalSize;
        ptmp += nalSize;
    }
    _cxt._used = pData - _cxt._data.get();
}
int h264_mp4toannexb::h264_mp4toannexb_filter(uint8_t **poutbuf, int *poutbuf_size,
                            const uint8_t *buf, int buf_size, const char *args /*= NULL*/)
{
    if (buf_size < 5)
        return -1;
    if (buf[4] == 0
            && buf[5] == 0
            && buf[6] == 0
            && buf[7] == 1)
    {
        toStand(buf, buf_size);
        buf = _cxt._data.get();
        buf_size = _cxt._used;
    }
    std::auto_ptr<AVCodecContext> avctx(new AVCodecContext);
    avctx->extradata = _extra_data.get();
    avctx->extradata_size = _extr_size;
    std::auto_ptr<H264BSFContext> ctx(new H264BSFContext);
    memset(ctx.get(), 0, sizeof(H264BSFContext));
    //ctx->extradata_parsed = 0;
    int i;
    uint8_t unit_type;
    int32_t nal_size;
    uint32_t cumul_size    = 0;
    const uint8_t *buf_end = buf + buf_size;
    int ret = 0;

    /* nothing to filter */
    if (_extr_size < 6) {
        *poutbuf      = (uint8_t *)buf;
        *poutbuf_size = buf_size;
        return 0;
    }

    /* retrieve sps and pps NAL units from extradata */
    if (!ctx->extradata_parsed) {
        if (args && strstr(args, "private_spspps_buf"))
            ctx->private_spspps = 1;

        ret = h264_extradata_to_annexb(ctx.get(), avctx.get(), AV_INPUT_BUFFER_PADDING_SIZE);
        if (ret < 0)
            return ret;
        ctx->length_size      = ret;
        ctx->new_idr          = 1;
        ctx->idr_sps_seen     = 0;
        ctx->idr_pps_seen     = 0;
        ctx->extradata_parsed = 1;
    }

    *poutbuf_size = 0;
    *poutbuf      = NULL;
    do {
        ret= AVERROR(EINVAL);
        if (buf + ctx->length_size > buf_end)
            goto fail;

        for (nal_size = 0, i = 0; i<ctx->length_size; i++)
            nal_size = (nal_size << 8) | buf[i];

        buf      += ctx->length_size;
        unit_type = *buf & 0x1f;

        if (nal_size > buf_end - buf || nal_size < 0)
            goto fail;

        if (unit_type == 7)
            ctx->idr_sps_seen = ctx->new_idr = 1;
        else if (unit_type == 8) {
            ctx->idr_pps_seen = ctx->new_idr = 1;
            /* if SPS has not been seen yet, prepend the AVCC one to PPS */
            if (!ctx->idr_sps_seen) {
                if (ctx->sps_offset == -1)
                    av_log(avctx.get(), AV_LOG_WARNING, "SPS not present in the stream, nor in AVCC, stream may be unreadable\n");
                else {
                    if ((ret = alloc_and_copy(poutbuf, poutbuf_size,
                                              ctx->spspps_buf + ctx->sps_offset,
                                              ctx->pps_offset != -1 ? ctx->pps_offset : ctx->spspps_size - ctx->sps_offset,
                                              buf, nal_size)) < 0)
                        goto fail;
                    ctx->idr_sps_seen = 1;
                    goto next_nal;
                }
            }
        }

        /* if this is a new IDR picture following an IDR picture, reset the idr flag.
         * Just check first_mb_in_slice to be 0 as this is the simplest solution.
         * This could be checking idr_pic_id instead, but would complexify the parsing. */
        if (!ctx->new_idr && unit_type == 5 && (buf[1] & 0x80))
            ctx->new_idr = 1;

        /* prepend only to the first type 5 NAL unit of an IDR picture, if no sps/pps are already present */
        if (ctx->new_idr && unit_type == 5 && !ctx->idr_sps_seen && !ctx->idr_pps_seen) {
            if ((ret=alloc_and_copy(poutbuf, poutbuf_size,
                                    ctx->spspps_buf, ctx->spspps_size,
                                    buf, nal_size)) < 0)
                goto fail;
            ctx->new_idr = 0;
            /* if only SPS has been seen, also insert PPS */
        } else if (ctx->new_idr && unit_type == 5 && ctx->idr_sps_seen && !ctx->idr_pps_seen) {
            if (ctx->pps_offset == -1) {
                av_log(avctx.get(), AV_LOG_WARNING, "PPS not present in the stream, nor in AVCC, stream may be unreadable\n");
                if ((ret = alloc_and_copy(poutbuf, poutbuf_size,
                                          NULL, 0, buf, nal_size)) < 0)
                    goto fail;
            } else if ((ret = alloc_and_copy(poutbuf, poutbuf_size,
                                             ctx->spspps_buf + ctx->pps_offset, ctx->spspps_size - ctx->pps_offset,
                                             buf, nal_size)) < 0)
                goto fail;
        } else {
            if ((ret=alloc_and_copy(poutbuf, poutbuf_size,
                                    NULL, 0, buf, nal_size)) < 0)
                goto fail;
            if (!ctx->new_idr && unit_type == 1) {
                ctx->new_idr = 1;
                ctx->idr_sps_seen = 0;
                ctx->idr_pps_seen = 0;
            }
        }

next_nal:
        buf        += nal_size;
        cumul_size += nal_size + ctx->length_size;
    } while (cumul_size < buf_size);
    av_free(ctx->spspps_buf);
    return 1;

fail:
    av_freep(poutbuf);
    *poutbuf_size = 0;
    av_free(ctx->spspps_buf);
    return ret;
}
int h264_mp4toannexb::h264_extradata_to_annexb(H264BSFContext *ctx, AVCodecContext *avctx, const int padding)
{
    uint16_t unit_size;
    uint64_t total_size                 = 0;
    uint8_t *out                        = NULL, unit_nb, sps_done = 0,
            sps_seen                   = 0, pps_seen = 0;
    const uint8_t *extradata            = avctx->extradata + 4;
    static const uint8_t nalu_header[4] = { 0, 0, 0, 1 };
    int length_size = (*extradata++ & 0x3) + 1; // retrieve length coded size

    ctx->sps_offset = ctx->pps_offset = -1;

    /* retrieve sps and pps unit(s) */
    unit_nb = *extradata++ & 0x1f; /* number of sps unit(s) */
    if (!unit_nb) {
        goto pps;
    } else {
        ctx->sps_offset = 0;
        sps_seen = 1;
    }

    while (unit_nb--) {
        int err;

        unit_size   = av_bswap16(*(uint16_t*)extradata);
        total_size += unit_size + 4;
        if (total_size > INT_MAX - padding) {
            av_log(avctx, AV_LOG_ERROR,
                   "Too big extradata size, corrupted stream or invalid MP4/AVCC bitstream\n");
            av_free(out);
            return AVERROR(EINVAL);
        }
        if (extradata + 2 + unit_size > avctx->extradata + avctx->extradata_size) {
            av_log(avctx, AV_LOG_ERROR, "Packet header is not contained in global extradata, "
                                        "corrupted stream or invalid MP4/AVCC bitstream\n");
            av_free(out);
            return AVERROR(EINVAL);
        }
        if ((err = av_reallocp(&out, total_size + padding)) < 0)
            return err;
        memcpy(out + total_size - unit_size - 4, nalu_header, 4);
        memcpy(out + total_size - unit_size, extradata + 2, unit_size);
        extradata += 2 + unit_size;
pps:
        if (!unit_nb && !sps_done++) {
            unit_nb = *extradata++; /* number of pps unit(s) */
            if (unit_nb) {
                ctx->pps_offset = total_size;
                pps_seen = 1;
            }
        }
    }

    if (out)
        memset(out + total_size, 0, padding);

    if (!sps_seen)
        av_log(avctx, AV_LOG_WARNING,
               "Warning: SPS NALU missing or invalid. "
               "The resulting stream may not play.\n");

    if (!pps_seen)
        av_log(avctx, AV_LOG_WARNING,
               "Warning: PPS NALU missing or invalid. "
               "The resulting stream may not play.\n");
    // wzd {
    //        if (!ctx->private_spspps) {
    //            av_free(avctx->extradata);
    //            avctx->extradata      = out;
    //            avctx->extradata_size = total_size;
    //        }
    // }
    ctx->spspps_buf  = out;
    ctx->spspps_size = total_size;

    return length_size;
}

int h264_mp4toannexb::alloc_and_copy(uint8_t **poutbuf, int *poutbuf_size,
                          const uint8_t *sps_pps, uint32_t sps_pps_size,
                          const uint8_t *in, uint32_t in_size)
{
    uint32_t offset         = *poutbuf_size;
    uint8_t nal_header_size = offset ? 3 : 4;
    int err;

    *poutbuf_size += sps_pps_size + in_size + nal_header_size;
    if ((err = av_reallocp(poutbuf,
                           *poutbuf_size + AV_INPUT_BUFFER_PADDING_SIZE)) < 0) {
        *poutbuf_size = 0;
        return err;
    }
    if (sps_pps)
        memcpy(*poutbuf + offset, sps_pps, sps_pps_size);
    memcpy(*poutbuf + sps_pps_size + nal_header_size + offset, in, in_size);
    if (!offset) {
        ((uint8_t*)(*poutbuf + sps_pps_size))[0] = 0;
        ((uint8_t*)(*poutbuf + sps_pps_size))[1] = 0;
        ((uint8_t*)(*poutbuf + sps_pps_size))[2] = 0;
        ((uint8_t*)(*poutbuf + sps_pps_size))[3] = 1;
    } else {
        (*poutbuf + offset + sps_pps_size)[0] =
                (*poutbuf + offset + sps_pps_size)[1] = 0;
        (*poutbuf + offset + sps_pps_size)[2] = 1;
    }

    return 0;
}

void *h264_mp4toannexb::av_realloc(void *ptr, size_t size)
{
#if CONFIG_MEMALIGN_HACK
    int diff;
#endif
static size_t max_alloc_size= INT_MAX;
    /* let's disallow possibly ambiguous cases */
    if (size > (max_alloc_size - 32))
        return NULL;

#if CONFIG_MEMALIGN_HACK
    //FIXME this isn't aligned correctly, though it probably isn't needed
    if (!ptr)
        return av_malloc(size);
    diff = ((char *)ptr)[-1];
    av_assert0(diff>0 && diff<=ALIGN);
    ptr = realloc((char *)ptr - diff, size + diff);
    if (ptr)
        ptr = (char *)ptr + diff;
    return ptr;
#elif HAVE_ALIGNED_MALLOC
    return _aligned_realloc(ptr, size + !size, ALIGN);
#else
    return realloc(ptr, size + !size);
#endif
}

int h264_mp4toannexb::av_reallocp(void *ptr, size_t size)
{
    void *val;

    if (!size) {
        av_freep(ptr);
        return 0;
    }

    memcpy(&val, ptr, sizeof(val));
    val = av_realloc(val, size);

    if (!val) {
        av_freep(ptr);
        return AVERROR(ENOMEM);
    }

    memcpy(ptr, &val, sizeof(val));
    return 0;
}
};
#include "get_bits.h"
#include "put_bits.h"
