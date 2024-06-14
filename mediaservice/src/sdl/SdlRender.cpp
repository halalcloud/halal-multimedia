#include "SdlRender.h"
CSdlRender::CStream::CStream(CSdlRender* pRender)
:m_pRender(pRender)
,m_pix_fmt(VMT_NONE)
,m_width(0)
,m_height(0)
,m_duration(0)
,m_ratioX(0)
,m_ratioY(0)
,m_window(NULL)
,m_render(NULL)
,m_texture(NULL)
{
}

CSdlRender::CStream::~CStream()
{
}

HRESULT CSdlRender::CStream::Init(IMediaType* pMT,size_t index)
{
    if(MMT_VIDEO == pMT->GetMajor())
    {
        if(MST_RAWVIDEO != pMT->GetSub())
        {
            pMT->SetSub(MST_RAWVIDEO);
        }
        uint32_t fourcc = pMT->GetFourcc(TAG_SDL_FLAG);
        if(0 == fourcc)
        {
            VideoMediaType vmt = VMT_YUV420P;
            pMT->SetVideoInfo(&vmt);
        }
    }
    else
    {
        JCHK1(false,false,"sdl render not support %s stream",pMT->GetMajorName());
    }
    JCHK(m_spPin.Create(CLSID_CInputPin,(IFilter*)m_pRender,false,&index),E_FAIL);
    return m_spPin->SetMediaType(pMT);
}

HRESULT CSdlRender::CStream::Check(IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        if(MMT_VIDEO == pMT->GetMajor())
        {
            if(MST_RAWVIDEO != pMT->GetSub())
                return E_INVALIDARG;
            JIF(pMT->GetVideoInfo(&m_pix_fmt,&m_width,&m_height,&m_ratioX,&m_ratioY,&m_duration));
        }
        else if(MMT_AUDIO == pMT->GetMajor())
        {
            //if(MST_PCM != pMT->GetSub())
            return E_INVALIDARG;
        }
        else
        {
            return  E_INVALIDARG;
        }
    }
    return hr;
}

HRESULT CSdlRender::CStream::Open()
{
    IMediaType* pMT;
    JCHK(NULL != (pMT = m_spPin->GetMediaType()),E_FAIL);
    if(MMT_VIDEO == pMT->GetMajor())
    {
        if(NULL == m_window)
        {
            JCHK1(m_window = SDL_CreateWindow("SDL video window",
                      SDL_WINDOWPOS_UNDEFINED,
                      SDL_WINDOWPOS_UNDEFINED,
                      m_width, m_height,
                      SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE/* SDL_WINDOW_HIDDEN| SDL_WINDOW_OPENGL*/),
                      E_FAIL,"SDL: could not set video mode - %s",SDL_GetError());
        }
        if(NULL == m_render)
        {
            JCHK1(m_render = SDL_CreateRenderer(m_window, -1, 0),E_FAIL,
                "SDL: could not create video render - %s",SDL_GetError());
        }
        if(NULL == m_texture)
        {
            uint32_t fourcc = pMT->GetFourcc(TAG_SDL_FLAG);
            //SDL_PIXELFORMAT_YUY2
            JCHK1(m_texture = SDL_CreateTexture(
                m_render,
                fourcc,
                SDL_TEXTUREACCESS_STREAMING,
                m_width,
                m_height),E_FAIL,
                "SDL: could not create video texture - %s",SDL_GetError());
        }
    }
    return S_OK;
}

HRESULT CSdlRender::CStream::Write(IMediaFrame* pFrame)
{
    HRESULT hr = S_OK;
    if(S_OK != hr)
        return hr;
    if(NULL == pFrame)
    {
        JIF(Close());
        return E_EOF;
    }
    else
    {
        JIF(Open());

        IMediaType* pMT;
        JCHK(pMT = m_spPin->GetMediaType(),E_FAIL);

        if(MMT_VIDEO == pMT->GetMajor())
        {
            SDL_Rect rect;
            rect.x = 0;
            rect.y = 0;
            rect.w = pFrame->info.stride;
            rect.h = m_height;
            const IMediaFrame::buf* buf = pFrame->GetBuf();
            SDL_UpdateTexture(m_texture,&rect, buf->data,pFrame->info.stride);
            SDL_RenderClear(m_render );
            SDL_RenderCopy(m_render,m_texture,&rect,&rect );
            SDL_RenderPresent(m_render );
        }
    }
    return S_OK;
}

HRESULT CSdlRender::CStream::Close()
{
    if(NULL != m_texture)
    {
        SDL_DestroyTexture(m_texture);
        m_texture = NULL;
    }
    if(NULL != m_render)
    {
        SDL_DestroyRenderer(m_render);
        m_render = NULL;
    }
    if(NULL != m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = NULL;
    }
    return S_OK;
}

bool CSdlRender::CStream::IsOpen()
{
    return NULL != m_texture;
}

CSdlRender::CSdlRender()
:m_pTag(NULL)
,m_status(S_Stop)
,m_open(false)
,m_expend_clock(MEDIA_FRAME_NONE_TIMESTAMP)
,m_expend_begin(MEDIA_FRAME_NONE_TIMESTAMP)
,m_expend_length(0)
{
    //ctor
}

bool CSdlRender::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_ep.Create(CLSID_CEventPoint,(IFilter*)this,true,pParam),false);
    JCHK(m_spProfile.Create(CLSID_CMemProfile,(IFilter*)this,true),false);
    return true;
}

bool CSdlRender::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Close();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CSdlRender)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(ILoad)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_ep)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP_(FilterType) CSdlRender::GetType()
{
    return FT_Render;
}

STDMETHODIMP CSdlRender::SetName(const char* pName)
{
    JCHK(NULL != pName,E_INVALIDARG);
    m_name = pName;
    return S_OK;
}

STDMETHODIMP_(const char*) CSdlRender::GetName()
{
    return m_name.c_str();
}

STDMETHODIMP_(uint32_t) CSdlRender::GetFlag()
{
    return FLAG_LIVE;
}

STDMETHODIMP_(uint32_t) CSdlRender::GetInputPinCount()
{
    return m_streams.size();
}

STDMETHODIMP_(IInputPin*) CSdlRender::GetInputPin(uint32_t index)
{
    return index < m_streams.size() ? m_streams[index]->m_spPin.p : NULL;
}

STDMETHODIMP_(uint32_t) CSdlRender::GetOutputPinCount()
{
    return 0;
}

STDMETHODIMP_(IOutputPin*) CSdlRender::GetOutputPin(uint32_t index)
{
    return NULL;
}

STDMETHODIMP_(IInputPin*) CSdlRender::CreateInputPin(IMediaType* pMT)
{
    JCHK(NULL != pMT,NULL);
    CStream* pStream;
    JCHK(pStream = new  CStream(this),NULL);
    shared_ptr<CStream> stream(pStream);
    JCHK(S_OK == pStream->Init(pMT,m_streams.size()),NULL);
    m_streams.push_back(stream);
    return pStream->m_spPin.p;
}

STDMETHODIMP_(IOutputPin*) CSdlRender::CreateOutputPin(IMediaType* pMT)
{
    return NULL;
}

STDMETHODIMP CSdlRender::Notify(uint32_t cmd)
{
    HRESULT hr = S_OK;
    if(cmd < IFilter::S_NB)
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
            m_status = (Status)cmd;
        }
        else
            return hr;
    }
    else if(cmd < C_NB)
    {
        if(C_Flush == cmd)
        {
        }
    }
    return hr;
}

STDMETHODIMP_(uint32_t) CSdlRender::GetStatus()
{
    return m_status;
}

STDMETHODIMP_(void) CSdlRender::SetTag(void* pTag)
{
    m_pTag = pTag;
}

STDMETHODIMP_(void*) CSdlRender::GetTag()
{
    return m_pTag;
}

STDMETHODIMP_(double) CSdlRender::GetExpend()
{
    int64_t now = GetTickCount();
    if(0 > m_expend_clock)
    {
        m_expend_clock = now;
        return 0;
    }
    else
    {
        int64_t duration = now - m_expend_clock;
        int64_t expend = 0;
        if(0 < m_expend_begin)
        {
            if(m_expend_begin < m_expend_clock)
                m_expend_length -= m_expend_clock - m_expend_begin;
            expend = now - m_expend_begin;
            m_expend_begin = now;
        }
        expend += m_expend_length;
        m_expend_length = 0;
        m_expend_clock = now;
        return expend * 100.0 / duration;
    }
}

STDMETHODIMP CSdlRender::OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT)
{
    uint32_t index = pPin->GetIndex();
    JCHK(index < m_streams.size(),E_INVALIDARG);
    return m_streams.at(index)->Check(pMT);
}

STDMETHODIMP CSdlRender::OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CSdlRender::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    HRESULT hr = S_OK;
    if(NULL != pPin && NULL != pFrame)
    {
        uint32_t index = pPin->GetIndex();
        JCHK(index < m_streams.size(),E_INVALIDARG);
        if(NULL != pFrame)
        {
            SDL_Event event;
            if(1 == SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                  case SDL_QUIT:
                    Close();
                    return E_EOF;
                  default:
                    break;
                }
            }
        }
        //m_ep->Notify(ET_Filter_Render,hr,pPin,pFrame);
        m_expend_begin = GetTickCount();
        hr = m_streams.at(index)->Write(pFrame);
        m_expend_length += GetTickCount() - m_expend_begin;
        m_expend_begin = MEDIA_FRAME_NONE_TIMESTAMP;
    }
    else
    {
        hr = E_EOF;
    }
    return hr;
}

STDMETHODIMP CSdlRender::Load(const char* pUrl,uint32_t mode,uint8_t flag)
{
    if(NULL != pUrl)
        m_name = pUrl;
    return S_OK;
}

HRESULT CSdlRender::Open()
{
    HRESULT hr = S_OK;
    if(false == m_open)
    {
        JCHK1(0 == SDL_Init(SDL_INIT_VIDEO /*| SDL_INIT_AUDIO */| SDL_INIT_TIMER),
            E_FAIL,"Could not initialize SDL - %s",SDL_GetError());
        m_open = true;
    }
    return hr;
}

HRESULT CSdlRender::Close()
{
    HRESULT hr = S_OK;
    if(true == m_open)
    {
        for(StreamIt it = m_streams.begin() ; it != m_streams.end() ; ++it)
            (*it)->Close();
        SDL_Quit();
        m_open = false;
    }
    return hr;
}

HRESULT CSdlRender::FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    return NULL != protocol && 0 == strcmp(protocol,"sdl") ? 1 : E_INVALIDARG;
}

