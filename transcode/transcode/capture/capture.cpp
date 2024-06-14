#include "capture.h"
#include <sys/ioctl.h>
#include "../src/Url.cpp"
#include <memory>
#include <time_expend.cpp>
using namespace std;
CCapture::CStream::CStream(CCapture* pCapture)
    :m_pCapture(pCapture)
    ,m_pMT(NULL)
    ,m_index(0)
    ,m_pix_fmt(VMT_NONE)
    ,m_width(0)
    ,m_height(0)
    ,m_ratioX(0)
    ,m_ratioY(0)
    ,m_duration(0)
    ,m_stride(0)
    ,m_bufs(NULL)
    ,m_time(0)
    ,m_delta(0)
    ,m_fd(INVALID_FD)
{
}

CCapture::CStream::~CStream()
{
    if(INVALID_FD == m_fd)
    {
        close(m_fd);
        m_fd = INVALID_FD;
    }
}

HRESULT CCapture::CStream::Set(const char* pName,IMediaType* pMT,size_t index)
{
    HRESULT hr = S_OK;
    if(MMT_VIDEO == pMT->GetMajor())
    {
        JCHK1(INVALID_FD != (m_fd = open(pName,O_RDWR|O_NONBLOCK)),E_FAIL,"can not open video device:%s",pName);

        v4l2_capability capb;
        JCHK3(0 <= ioctl(m_fd,VIDIOC_QUERYCAP,&capb),E_INVALIDARG,
            "can not get video device:%s capability return:%d msg:%s",pName,errno,strerror(errno));
        LOG(0,"video capture device:%s drive:[%s:%u.%u.%u]card:[%s] bus:[%s]",
            pName,
            capb.driver,(capb.version>>16)&0XFF,
            (capb.version>>8)&0XFF,capb.version&0XFF,capb.card,capb.bus_info);

        v4l2_cropcap cap;
        memset(&cap,0,sizeof(cap));
        cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        JCHK1(-1 != ioctl(m_fd, VIDIOC_CROPCAP, &cap),E_FAIL,
              "can not get video device:%s video capacity",pName);

        v4l2_format fmt;
        memset(&fmt,0,sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        JCHK1(-1 != ioctl(m_fd, VIDIOC_G_FMT, &fmt),E_FAIL,
              "can not get video device:%s fmt",pName);

        v4l2_streamparm parm;
        memset(&parm,0,sizeof(parm));
        parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        JCHK1(-1 != ioctl(m_fd, VIDIOC_G_PARM, &parm),E_FAIL,
              "can not get device:[%s] video param",pName);

        m_duration = 400000;
        if(0 != (V4L2_CAP_TIMEPERFRAME & parm.parm.capture.capability) &&
                0 != parm.parm.capture.timeperframe.numerator &&
                0 != parm.parm.capture.timeperframe.denominator)
        {
            double fps = double(parm.parm.capture.timeperframe.denominator) / parm.parm.capture.timeperframe.numerator;
            m_duration = int64_t(10000000.0 / fps + 0.5);
        }

        JIF(pMT->SetSub(MST_RAWVIDEO));

        m_pix_fmt = VMT_YUV420P;
        m_width = fmt.fmt.pix.width,m_height = fmt.fmt.pix.height,m_ratioX = m_width,m_ratioY = m_height;
        pMT->GetVideoInfo(&m_pix_fmt,&m_width,&m_height,&m_ratioX,&m_ratioY,&m_duration);
        if(0 < cap.bounds.width && m_width > cap.bounds.width)
            m_width = cap.bounds.width;
        if(0 < cap.bounds.height && m_height > cap.bounds.height)
            m_height = cap.bounds.height;
        JIF(pMT->SetVideoInfo(&m_pix_fmt,&m_width,&m_height,&m_ratioX,&m_ratioY,&m_duration));

        JCHK1(0 != (fmt.fmt.pix.pixelformat = pMT->GetFourcc(TAG_V4L2_FLAG)),E_FAIL,
              "pixel format:%d convert to fourcc fail",m_pix_fmt);
        fmt.fmt.pix.width = m_width;
        fmt.fmt.pix.height = m_height;
        JCHK4(-1 != ioctl(m_fd, VIDIOC_S_FMT, &fmt),E_FAIL,
              "video device:%s set video format:[fourcc:0x%x width:%d height:%d] fail",
              pName,fmt.fmt.pix.pixelformat,m_width,m_height);
        m_stride = fmt.fmt.pix.bytesperline;
        if(0 != (V4L2_CAP_TIMEPERFRAME & parm.parm.capture.capability))
        {
            parm.parm.capture.timeperframe.numerator = m_duration;
            parm.parm.capture.timeperframe.denominator = 10000000;
            JCHK1(-1 != ioctl(m_fd, VIDIOC_S_PARM, &parm),E_FAIL,
                  "can not set video device:%s param",pName);
        }
    }
    else if(MMT_AUDIO == pMT->GetMajor())
    {
        AudioMediaType fmt = AMT_S16;
        int ch = 2;
        int sr = 44100;
        int fs = 1024;
        pMT->GetAudioInfo(&fmt, NULL, &ch, &sr, &fs);
// save parameter
        if (fmt != AMT_S16)
            fmt = AMT_S16;
        if (ch != 2)
            ch = 2;
        if (sr != 44100)
            sr = 44100;
        if (fs <= 0)
            fs = 1024;
        _fs = fs;
        JIF(pMT->SetSub(MST_PCM));
        JIF(pMT->SetAudioInfo(&fmt, NULL, &ch, &sr, &fs));
        _frequency = sr;
        m_duration = int64_t(fs * 10000000.0 / sr + 0.5);
    }
    JCHK(m_spPin.Create(CLSID_COutputPin,(IFilter*)m_pCapture,&index),E_FAIL);
    m_spMT = pMT;
    m_name = pName;
    return hr;
}

HRESULT CCapture::CStream::Open()
{
    HRESULT hr ;
    if(NULL == m_pMT)
        return S_OK;
    if(MMT_VIDEO == m_pMT->GetMajor())
    {
        v4l2_requestbuffers req;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.count = VIDEO_BUFFER_COUNT;
        req.memory =V4L2_MEMORY_MMAP;
        JCHK1(-1 != ioctl(m_fd,VIDIOC_REQBUFS,&req),E_FAIL,
              "device:[%s] request video 4 frames buffer fail",
              m_name.c_str());
        JCHK(m_bufs = new buffer[VIDEO_BUFFER_COUNT],E_OUTOFMEMORY);
        for(uint32_t i=0 ; i<VIDEO_BUFFER_COUNT ; ++i)
        {
            struct v4l2_buffer buf;
            memset(&buf,0,sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            JCHK2(-1 != ioctl (m_fd, VIDIOC_QUERYBUF, &buf),E_FAIL,
                  "device:[%s] query frame buffer[:%d] fail",
                  m_name.c_str(),i);
            m_bufs[i].size = buf.length;
            JCHK2(m_bufs[i].data = (uint8_t*)mmap(NULL,buf.length,PROT_READ | PROT_WRITE,MAP_SHARED,m_fd, buf.m.offset),
                  E_FAIL,"device:[%s] map frame buffer[:%d] fail",
                  m_name.c_str(),i);
        }
        for(uint32_t i=0 ; i<VIDEO_BUFFER_COUNT ; ++i)
        {
            struct v4l2_buffer buf;
            memset(&buf,0,sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            JCHK2(-1 != ioctl (m_fd, VIDIOC_QBUF, &buf),E_FAIL,
                  "device:[%s] push frame buffer[:%d] fail",
                  m_name.c_str(),i);
        }
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        JCHK1(-1 != ioctl(m_fd,VIDIOC_STREAMON,&type),E_FAIL,
              "device:[%s] start video capture fail",
              m_name.c_str());
        JCHK(m_spFrame.Create(CLSID_CMediaFrame),E_FAIL);
        m_spFrame->info.stride = m_stride;
        m_spFrame->info.duration = m_duration;

//        v4l2_audio a;
//        memset(&a, 0, sizeof(v4l2_audio));
//        if (-1 == ioctl(m_fd, VIDIOC_G_AUDIO, &a))
//        {
//
//            printf("VIDIOC_G_AUDIO\n");
//        }
//        printf("current input: %s \t %d\n", a.name, a.index);
//        if (-1 == ioctl(m_fd, VIDIOC_S_AUDIO, &a))
//        {
//            printf("VIDIOC_S_AUDIO\n");
//        }
    }
    else if (MMT_AUDIO == m_pMT->GetMajor())
    {
        // init audio device
        snd_pcm_hw_params_t *hw_params = NULL;
        JCHK2((hr = snd_pcm_open (&_capture_handle, m_name.c_str(), SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK)) >= 0, E_FAIL,
              "cannot open audio device %s (%s)", m_name.c_str(), snd_strerror(hr) );


        JCHK2((hr = snd_pcm_hw_params_malloc (&hw_params)) >= 0, E_FAIL,
              "audio device:%s cannot allocate hardware parameter structure (%s)",
              m_name.c_str(),snd_strerror (hr) );

        JCHK2((hr = snd_pcm_hw_params_any (_capture_handle, hw_params)) >= 0, E_FAIL,
              "audio device:%s cannot initialize hardware parameter structure (%s)",
              m_name.c_str(),snd_strerror (hr));


        JCHK2((hr = snd_pcm_hw_params_set_access (_capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) >= 0, E_FAIL,
              "audio device:%s cannot set access type (%s)",
              m_name.c_str(),snd_strerror (hr));


        JCHK2 ((hr = snd_pcm_hw_params_set_format (_capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) >= 0, E_FAIL,
               "audio device:%s cannot set sample format (%s)",
               m_name.c_str(),snd_strerror (hr));


        JCHK2 ((hr = snd_pcm_hw_params_set_rate_near (_capture_handle, hw_params, &_frequency, 0)) >= 0, E_FAIL,
               "audio device:%s cannot set sample rate (%s)",
               m_name.c_str(),snd_strerror (hr));


        JCHK2 ((hr = snd_pcm_hw_params_set_channels (_capture_handle, hw_params, 2)) >= 0, E_FAIL,
               "audio device:%s cannot set channel count (%s)",
               m_name.c_str(),snd_strerror (hr));

        JCHK2 ((hr = snd_pcm_hw_params (_capture_handle, hw_params)) >= 0, E_FAIL,
               "audio device:%s cannot set parameters (%s)",
               m_name.c_str(),snd_strerror (hr));
        int dir;
        /* Set period size to 32 frames. */
        snd_pcm_uframes_t frames = _fs*4;
        JCHK3(0 <= snd_pcm_hw_params_set_period_size_near(_capture_handle,
                                               hw_params, &frames, &dir),E_FAIL,
              "audio device:%s unable to set hw sample buf size:%d msg:%s",m_name.c_str(),frames,snd_strerror(hr));

        /* Write the parameters to the driver */
        JCHK2((hr = snd_pcm_hw_params(_capture_handle, hw_params)) >= 0, E_FAIL,
              "audio device:%s unable to set hw parameters: %s",
              m_name.c_str(),snd_strerror(hr));


        /* Use a buffer large enough to hold one period */
//        snd_pcm_hw_params_get_period_size(hw_params,
//                                          &frames, &dir);
//        if (_fs > (int)frames)
//            _bufLen = _fs * 4;
//        else
//            _bufLen = frames * 4;
        _bufLen = _fs * 4;
        snd_pcm_hw_params_free (hw_params);

        JCHK2 ((hr = snd_pcm_prepare (_capture_handle)) >= 0, E_FAIL,
               "audio device:%s cannot prepare audio interface for use (%s)\n",
               m_name.c_str(),snd_strerror (hr));
#ifdef WRITE_PCM
_fp = fopen("pcm.cpm", "wb");
#endif // WRITE_PCM

    }
    m_time = MEDIA_FRAME_NONE_TIMESTAMP;
    m_delta = 0;
    return S_OK;
}

HRESULT CCapture::CStream::Process()
{
    HRESULT hr = S_OK;
    if(MMT_VIDEO == m_pMT->GetMajor())
    {
        v4l2_buffer buf;
        memset(&buf,0,sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        {
            time_expend te(m_pCapture->m_te);
            hr = ioctl(m_fd, VIDIOC_DQBUF, &buf);
            if(-1 == hr)
            {
                if(EAGAIN == errno)
                {
                    usleep(m_duration/10);
                    m_delta += m_duration;
                    LOG(0,"video device:%s read empty",m_name.c_str());
                    return S_FALSE;
                }
                JCHK3(-1 != hr,E_FAIL,
                    "video device:%s pop video frame fail id:%d,msg:%s",
                    m_name.c_str(),errno,strerror(errno));
            }
        }
        JIF(m_spFrame->SetBuf(m_bufs[buf.index].data,m_bufs[buf.index].size));

        uint8_t *data[4];
        int linesize[4];
        JIF(m_spMT->FrameToArray(data,linesize,m_spFrame));

        dom_ptr<IMediaFrame> spFrame;
        JIF(m_spPin->AllocFrame(&spFrame));

        spFrame->info.dts = buf.timestamp.tv_sec * 10000000 + buf.timestamp.tv_usec * 10;

        if(MEDIA_FRAME_NONE_TIMESTAMP == m_time)
        {
            if(MEDIA_FRAME_NONE_TIMESTAMP == m_pCapture->m_start)
            {
                m_pCapture->m_start = spFrame->info.dts;
                //m_delta = m_pCapture->m_start - spFrame->info.dts;
            }
        }
        spFrame->info.dts += m_delta;

        JIF(m_spMT->ArrayToFrame(spFrame,(const uint8_t**)data,linesize));

        JCHK3(-1 != ioctl(m_fd,VIDIOC_QBUF,&buf),E_FAIL,
              "video device:%s pop video frame fail id:%d,msg:%s",
              m_name.c_str(),errno,strerror(errno));

        return Output(spFrame);
    }
    else if (MMT_AUDIO == m_pMT->GetMajor())
    {
// read audio data from device
        if(m_spFrame == NULL)
        {
            JIF(m_spPin->AllocFrame(&m_spFrame));
            m_spFrame->Alloc(_bufLen);
            m_spFrame->info.samples = 0;
        }
        {
            time_expend te(m_pCapture->m_te);
            do
            {
                uint8_t* pBuf = m_spFrame->GetBuf() + m_spFrame->info.samples*4;
                hr = snd_pcm_readi (_capture_handle, pBuf, (snd_pcm_uframes_t)(_fs-m_spFrame->info.samples));
                if(-EAGAIN == hr)
                {
                    usleep(m_duration/10);
                    m_time = MEDIA_FRAME_NONE_TIMESTAMP;
                    return S_FALSE;
                }
                else if(-EPIPE == hr)
                {
                    LOG(1,"audio device:%s capture return EPIPE",m_name.c_str());
                    JCHK3(0 <= (hr = snd_pcm_prepare(_capture_handle)),E_FAIL,
                         "audio device:%s cannot prepare audio interface for use return%d msg:(%s)",
                         m_name.c_str(),hr,snd_strerror(hr));
                    m_spFrame->info.samples = 0;
                    m_time = MEDIA_FRAME_NONE_TIMESTAMP;
                }
                else
                {
                    JCHK3(0 <= hr,E_FAIL,"audio device:%s read from audio interface failed return:%d msg:(%s)",m_name.c_str(),hr,snd_strerror(hr));
                    m_spFrame->info.samples += hr;
                }
            }while(m_spFrame->info.samples < _fs);

        }
        JIF(m_spFrame->SetBuf(NULL, _bufLen));
        if(MEDIA_FRAME_NONE_TIMESTAMP == m_time)
        {
            m_spFrame->info.dts = GetTickCount();
            if(MEDIA_FRAME_NONE_TIMESTAMP == m_pCapture->m_start)
            {
                m_pCapture->m_start = m_spFrame->info.dts;
            }
        }
        else
            m_spFrame->info.dts = m_time + m_duration;

        hr = Output(m_spFrame);
        m_spFrame = NULL;
        #ifdef WRITE_PCM
        fwrite(spFrame->GetBuf(), 1, _bufLen, _fp);
        #endif // WRITE_PCM
    }
    return hr;
}

HRESULT CCapture::CStream::Output(IMediaFrame* pFrame)
{
    if(NULL != pFrame)
    {
        m_time = pFrame->info.dts;
        if(MEDIA_FRAME_NONE_TIMESTAMP != m_pCapture->m_begin)
            pFrame->info.dts += m_pCapture->m_begin - m_pCapture->m_start;
        pFrame->info.pts = pFrame->info.dts;
        pFrame->info.duration = m_duration;
        pFrame->info.flag = MEDIA_FRAME_FLAG_SYNCPOINT;
        m_pCapture->m_time = pFrame->info.dts;
    }
    //printf("%s frame PTS:%ld DTS:%ld output\n",m_spMT->GetMajorName(),pFrame->info.pts,pFrame->info.dts);
    return m_spPin->Write(pFrame);
}

HRESULT CCapture::CStream::Close()
{
    if(NULL == m_pMT)
        return S_OK;
    if(MMT_VIDEO == m_pMT->GetMajor())
    {
        m_spFrame = NULL;
        JCHK1(-1 != ioctl(m_fd,VIDIOC_STREAMOFF,NULL),E_FAIL,
              "video device:%s stop capture fail",
              m_name.c_str());
        if(NULL != m_bufs)
        {
            for(uint32_t i=0 ; i<VIDEO_BUFFER_COUNT ; ++i)
            {
                if(NULL != m_bufs[i].data && 0 < m_bufs[i].size)
                    munmap(m_bufs[i].data,m_bufs[i].size);
            }
            delete[] m_bufs;
            m_bufs = NULL;
        }
    }
    else if (MMT_AUDIO == m_pMT->GetMajor())
    {
        m_spFrame = NULL;
        snd_pcm_close(_capture_handle);
    }
    #ifdef WRITE_PCM
    fclose(_fp);
    #endif // WRITE_PCM
    return S_OK;
}

CCapture::CCapture()
    :m_pFG(NULL)
    ,m_tag(NULL)
    ,m_isOpen(false)
    ,m_isEOF(false)
    ,m_begin(MEDIA_FRAME_NONE_TIMESTAMP)
    ,m_start(MEDIA_FRAME_NONE_TIMESTAMP)
    ,m_time(MEDIA_FRAME_NONE_TIMESTAMP)
    ,m_delta(0)
    ,m_pMaster(NULL)
{
    //ctor
    memset(m_index,0,sizeof(m_index));
}

bool CCapture::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_pFG = static_cast<IFilterGraphEvent*>(pParam),false);
    return m_spProfile.Create(CLSID_CMemProfile,dynamic_cast<IFilter*>(this)) ;
}

bool CCapture::FinalDestructor(bool finally)
{
    if(true == finally)
        Clear();
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CCapture)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(IDemuxer)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CCapture::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CCapture::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CCapture::GetInputPinCount()
{
    return 0;
}

STDMETHODIMP_(IInputPin*) CCapture::GetInputPin(uint32_t index)
{
    return NULL;
}

STDMETHODIMP_(uint32_t) CCapture::GetOutputPinCount()
{
    return m_streams.size();
}

STDMETHODIMP_(IOutputPin*) CCapture::GetOutputPin(uint32_t index)
{
    JCHK(index < m_streams.size(),NULL);
    return m_streams[index]->m_spPin.p;
}

STDMETHODIMP CCapture::Open()
{
    HRESULT hr = S_OK;
    if(false == m_isOpen)
    {
        for(StreamIt it = m_streams.begin() ; it != m_streams.end() ; ++it)
            (*it)->Open();
        //__useconds_t us = m_duration/10;
        //sleep(1);
        m_start = MEDIA_FRAME_NONE_TIMESTAMP;
        m_time = MEDIA_FRAME_NONE_TIMESTAMP;
        m_delta = 0;
        m_isEOF = false;
        m_isOpen = true;
        hr = m_pFG->Notify(IFilterEvent::Open,hr,dynamic_cast<IFilter*>(this));
    }
    return hr;
}

STDMETHODIMP CCapture::Close()
{
    HRESULT hr = S_OK;
    if(true == m_isOpen)
    {
        for(StreamIt it = m_streams.begin() ; it != m_streams.end() ; ++it)
            (*it)->Close();

        m_isOpen = false;
        hr = m_pFG->Notify(IFilterEvent::Close,hr,this);
    }
    return hr;
}

STDMETHODIMP CCapture::SetTag(void* pTag)
{
    m_tag = pTag;
    return S_OK;
}

STDMETHODIMP_(double) CCapture::GetExpend()
{
    return m_te.get();
}

STDMETHODIMP_(void*) CCapture::GetTag()
{
    return m_tag;
}

STDMETHODIMP CCapture::OnGetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CCapture::OnGetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    size_t index = pPin->GetIndex();
    JCHK(index < m_streams.size(),E_INVALIDARG);
    return pMT->CopyFrom(m_streams[index]->m_spMT);
}

STDMETHODIMP CCapture::OnSetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CCapture::OnSetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    HRESULT hr = S_OK;
    size_t index = pPin->GetIndex();
    JCHK(index < m_streams.size(),E_INVALIDARG);
    if(NULL  != pMT)
    {
        JIF(m_streams[index]->m_spMT->Compare(pMT));
    }
    if(S_OK == hr)
        m_streams[index]->m_pMT = pMT;
    else
        hr = E_INVALIDARG;
    return hr;
}

STDMETHODIMP CCapture::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    return E_FAIL;
}

STDMETHODIMP CCapture::OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame)
{
    return m_pFG->Notify(type,hr,this,pPinIn,pPinOut,pFrame);
}

STDMETHODIMP CCapture::Load(const char* pUrl)
{
    JCHK(NULL != pUrl,E_INVALIDARG);

    Clear();
    return SetName(pUrl);
}

STDMETHODIMP_(uint32_t) CCapture::GetFlag()
{
    return FILTER_FLAG_LIVE;
}

STDMETHODIMP_(IOutputPin*) CCapture::CreatePin(IMediaType* pMT)
{
    if(MMT_VIDEO == pMT->GetMajor() || MMT_AUDIO == pMT->GetMajor())
    {

        dom_ptr<IProfile> spProfile(this);
        IProfile::val* pVal = spProfile->Read(pMT->GetMajorName());
        JCHK2(NULL != pVal,NULL,"capture:%s not set %s device",m_name.c_str(),pMT->GetMajorName());
        JCHK0(true == STR_CMP(pVal->type,typeid(const char*).name())||
              true == STR_CMP(pVal->type,typeid(char*).name()),
              NULL,"capture:%s %s device type not string");

        CStream* pStream;
        JCHK(pStream = new CStream(this),NULL);
        HRESULT hr = pStream->Set(static_cast<const char*>(pVal->value),pMT,m_streams.size());
        if(S_OK != hr)
        {
            delete pStream;
            pStream = NULL;
        }
        else
            m_streams.push_back(pStream);
        if(NULL == m_pMaster)
            m_pMaster = pStream;
        return NULL == pStream ? NULL : pStream->m_spPin;
    }
    else
    {
        JCHK2(false,NULL,"capture:%s not support %s stream",m_name.c_str(),pMT->GetMajorName());
    }
    return NULL;
}

STDMETHODIMP_(void) CCapture::SetStartTime(const int64_t& time)
{
    m_begin = time;
}

STDMETHODIMP_(int64_t) CCapture::GetTime()
{
    return m_time;
}

STDMETHODIMP CCapture::Process()
{
    HRESULT hr = S_OK;

    if(false == m_isOpen)
        hr = Open();

    while(IS_OK(hr) && S_STREAM_EOF != hr && false == m_pFG->IsExit())
    {
        int64_t min = MEDIA_FRAME_NONE_TIMESTAMP;
        CStream* pStream = NULL;
        for(StreamIt it = m_streams.begin() ; it != m_streams.end() ; ++it)
        {
            CStream* pTempStream = *it;
            if(NULL != pTempStream->m_pMT)
            {
                if(NULL == pStream || pTempStream->m_time < min)
                {
                    pStream = pTempStream;
                    min = pStream->m_time;
                }
            }
        }
        if(NULL != pStream)
        {
            JIF(pStream->Process());
            if(S_OK == hr)
                return hr;
            else
                continue;
        }
    }
    LOG(0,"url:[%s] demux begin flush hr:%d",m_name.c_str(),hr);
    hr = m_pFG->Notify(IFilterEvent::FlushBegin,hr,dynamic_cast<IFilter*>(this));
    if(S_STREAM_EOF == hr)
    {
        bool isEOF;
        do
        {
            isEOF = true;
            for(StreamIt it = m_streams.begin() ; it != m_streams.end() ; ++it)
            {
                CStream* pStream = *it;
                if(NULL != pStream->m_pMT)
                {
                    if(S_STREAM_EOF != pStream->Output(NULL))
                        isEOF = false;
                }
            }
        }
        while(false == isEOF);
        hr = m_pFG->Notify(IFilterEvent::FlushEnd,hr,dynamic_cast<IFilter*>(this));
        if(S_STREAM_EOF == hr)
            m_isEOF = isEOF;
        LOG(0,"url:[%s] demux end flush",m_name.c_str());
    }
    Close();
    return hr;
}

STDMETHODIMP_(void) CCapture::Clear()
{
    Close();
    m_pMaster = NULL;
    for(StreamIt it = m_streams.begin() ; it != m_streams.end() ; ++it)
    {
        delete *it;
    }
    m_streams.clear();
}

STDMETHODIMP_(bool) CCapture::IsEOF()
{
    return m_isEOF;
}

STDMETHODIMP_(void) CCapture::NewSegment()
{
    for(size_t i = 0 ; i < m_streams.size() ; ++i)
        m_streams[i]->m_spPin->NewSegment();
}

HRESULT CCapture::UrlSupportQuery(const char* pProtocol,const char* pFormat)
{
    return NULL != pProtocol && 0 == strcmp(pProtocol,"capture") ? 1 : E_INVALIDARG;
}

