#include "SdlRender.h"
CSdlRender::CStream::CStream(CSdlRender* pRender)
:m_pRender(pRender)
,m_pMT(NULL)
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

bool CSdlRender::CStream::Init(IMediaType* pMT,size_t index)
{
    if(MMT_VIDEO == pMT->GetMajor())
    {
        if(MST_RAWVIDEO != pMT->GetSub())
        {
            pMT->SetSub(MST_RAWVIDEO);
        }
    }
    else
    {
        JCHK1(false,false,"sdl render not support %s stream",pMT->GetMajorName());
    }
    JCHK(m_spPin.Create(CLSID_CInputPin,(IFilter*)m_pRender,&index),NULL);
    m_spMT = pMT;
    return true;
}

HRESULT CSdlRender::CStream::Set(IMediaType* pMT)
{
    HRESULT hr = S_OK;
    if(NULL != pMT)
    {
        if(m_spMT->GetSub() != pMT->GetSub())
            return E_INVALIDARG;

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
    m_pMT = pMT;
    return hr;
}

HRESULT CSdlRender::CStream::Open()
{
    JCHK(NULL != m_pMT,E_FAIL);
    if(MMT_VIDEO == m_pMT->GetMajor())
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
            uint32_t fourcc = m_pMT->GetFourcc(TAG_SDL_FLAG);
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
    hr = m_pRender->m_pFG->Notify(IFilterEvent::Process,hr,m_pRender,m_spPin,NULL,pFrame);
    if(S_OK != hr)
        return hr;
    if(NULL == pFrame)
    {
        JIF(Close());
        return S_STREAM_EOF;
    }
    else
    {
        JIF(Open());
        if(MMT_VIDEO == m_pMT->GetMajor())
        {
            SDL_Rect rect;
            rect.x = 0;
            rect.y = 0;
            rect.w = pFrame->info.stride;
            rect.h = m_height;
            SDL_UpdateTexture(m_texture,&rect, pFrame->GetBuf(),pFrame->info.stride);
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
:m_pFG(NULL)
,m_tag(NULL)
,m_open(false)
,m_expend_clock(MEDIA_FRAME_NONE_TIMESTAMP)
,m_expend_begin(MEDIA_FRAME_NONE_TIMESTAMP)
,m_expend_length(0)
{
    //ctor
}

bool CSdlRender::FinalConstruct(Interface* pOuter,void* pParam)
{
    JCHK(m_pFG = static_cast<IFilterGraphEvent*>(pParam),false);
    return m_spProfile.Create(CLSID_CMemProfile,dynamic_cast<IFilter*>(this)) ;
}

bool CSdlRender::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Clear();
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CSdlRender)
DOM_QUERY_IMPLEMENT(IFilter)
DOM_QUERY_IMPLEMENT(IMuxer)
DOM_QUERY_IMPLEMENT_AGGREGATE(m_spProfile)
DOM_QUERY_IMPLEMENT_END

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
    return FILTER_FLAG_LIVE;
}

STDMETHODIMP_(uint32_t) CSdlRender::GetInputPinCount()
{
    return m_streams.size();
}

STDMETHODIMP_(IInputPin*) CSdlRender::GetInputPin(uint32_t index)
{
    JCHK(index < m_streams.size(),NULL);
    return m_streams[index]->m_spPin.p;
}

STDMETHODIMP_(uint32_t) CSdlRender::GetOutputPinCount()
{
    return m_streams.size();
}

STDMETHODIMP_(IOutputPin*) CSdlRender::GetOutputPin(uint32_t index)
{
    return 0;
}

STDMETHODIMP CSdlRender::Open()
{
    HRESULT hr = S_OK;
    if(false == m_open)
    {
        JCHK1(0 == SDL_Init(SDL_INIT_VIDEO /*| SDL_INIT_AUDIO */| SDL_INIT_TIMER),
            E_FAIL,"Could not initialize SDL - %s",SDL_GetError());
        m_open = true;
        return m_pFG->Notify(IFilterEvent::Open,hr,this);
    }
    return hr;
}

STDMETHODIMP CSdlRender::Close()
{
    HRESULT hr = S_OK;
    if(true == m_open)
    {
        for(StreamIt it = m_streams.begin() ; it != m_streams.end() ; ++it)
            (*it)->Close();
        SDL_Quit();
        m_open = false;
        hr = m_pFG->Notify(IFilterEvent::Close,hr,this);
    }
    return hr;
}

STDMETHODIMP CSdlRender::SetTag(void* pTag)
{
    m_tag = pTag;
    return S_OK;
}

STDMETHODIMP_(void*) CSdlRender::GetTag()
{
    return m_tag;
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

STDMETHODIMP CSdlRender::OnGetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    JCHK(NULL != pPin,E_INVALIDARG);
    JCHK(NULL != pMT,E_INVALIDARG);

    CStream* pStream;
    size_t index = pPin->GetIndex();
    JCHK(pStream = m_streams[index],E_INVALIDARG);

    return pMT->CopyFrom(pStream->m_spMT,COPY_FLAG_COMPILATIONS|COPY_FLAG_INTERSECTION);
}

STDMETHODIMP CSdlRender::OnGetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CSdlRender::OnSetMediaType(IInputPin* pPin,IMediaType* pMT)
{
    uint32_t index = pPin->GetIndex();
    JCHK(index < m_streams.size(),E_INVALIDARG);
    CStream* pStream = m_streams.at(index);
    return pStream->Set(pMT);
}

STDMETHODIMP CSdlRender::OnSetMediaType(IOutputPin* pPin,IMediaType* pMT)
{
    return E_FAIL;
}

STDMETHODIMP CSdlRender::OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame)
{
    HRESULT hr = S_OK;
    uint32_t index = pPin->GetIndex();
    JCHK(index < m_streams.size(),E_INVALIDARG);
    CStream* pStream = m_streams.at(index);
    if(NULL != pFrame)
    {
        SDL_Event event;
        if(1 == SDL_PollEvent(&event))
        {
            switch(event.type)
            {
              case SDL_QUIT:
                Close();
                return S_STREAM_EOF;
              default:
                break;
            }
        }
        JIF(Open());
    }

    m_pFG->Notify(IFilterEvent::Process,S_OK,this,pStream->m_spPin,NULL,pFrame);

    m_expend_begin = GetTickCount();
    hr = pStream->Write(pFrame);
    m_expend_length += GetTickCount() - m_expend_begin;
    m_expend_begin = MEDIA_FRAME_NONE_TIMESTAMP;
    if(S_STREAM_EOF == hr)
    {
        for(StreamIt it=m_streams.begin() ; it != m_streams.end() ; ++it)
        {
            if(true == (*it)->IsOpen())
                return hr;
        }
        Close();
    }
    return hr;
}

STDMETHODIMP CSdlRender::OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame)
{
    return m_pFG->Notify(type,hr,this,pPinIn,pPinOut,pFrame);
}

STDMETHODIMP CSdlRender::Load(const char* pUrl)
{
    JCHK(NULL != pUrl,E_INVALIDARG);
    m_name = pUrl;
    return S_OK;
}

STDMETHODIMP_(IInputPin*) CSdlRender::CreatePin(IMediaType* pMT)
{
    JCHK(NULL != pMT,NULL);
    CStream* pStream;
    JCHK(pStream = new  CStream(this),NULL);
    if(false == pStream->Init(pMT,m_streams.size()))
    {
        delete pStream;
        return NULL;
    }
    m_streams.push_back(pStream);
    return pStream->m_spPin;
}

STDMETHODIMP_(void) CSdlRender::Clear()
{
    Close();
}

HRESULT CSdlRender::UrlSupportQuery(const char* pProtocol,const char* pFormat)
{
    return NULL != pProtocol && 0 == strcmp(pProtocol,"sdl") ? 1 : E_INVALIDARG;
}

