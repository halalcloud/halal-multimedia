#ifndef SDLRENDER_H
#define SDLRENDER_H
#include "stdafx.h"

class CSdlRender : public IFilter , public IMuxer
{
    class CStream
    {
        friend class CSdlRender;
    public:
        CStream(CSdlRender* pRender);
        virtual ~CStream();
        bool Init(IMediaType* pMT,size_t index);
        HRESULT Set(IMediaType* pMT);
        HRESULT Open();
        HRESULT Write(IMediaFrame* pFrame);
        HRESULT Close();
        bool IsOpen();
    protected:
        CSdlRender* m_pRender;
        dom_ptr<IInputPin> m_spPin;
        dom_ptr<IMediaType> m_spMT;
        IMediaType* m_pMT;
        VideoMediaType m_pix_fmt;
        int m_width;
        int m_height;
        int64_t m_duration;
        int m_ratioX;
        int m_ratioY;
        SDL_Window* m_window;
        SDL_Renderer* m_render;
        SDL_Texture* m_texture;
    };
    friend class CStream;
    typedef vector<CStream*> StreamSet;
    typedef StreamSet::iterator StreamIt;
public:
    DOM_DECLARE(CSdlRender)
    //IFilter
    STDMETHODIMP SetName(const char* pName);
    STDMETHODIMP_(const char*) GetName();
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP_(uint32_t) GetInputPinCount();
    STDMETHODIMP_(IInputPin*) GetInputPin(uint32_t index);
    STDMETHODIMP_(uint32_t) GetOutputPinCount();
    STDMETHODIMP_(IOutputPin*) GetOutputPin(uint32_t index);
    STDMETHODIMP Open();
    STDMETHODIMP Close();
    STDMETHODIMP SetTag(void* pTag);
    STDMETHODIMP_(void*) GetTag();
    STDMETHODIMP_(double) GetExpend();
    STDMETHODIMP OnGetMediaType(IInputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnGetMediaType(IOutputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnSetMediaType(IInputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnSetMediaType(IOutputPin* pPin,IMediaType* pMT);
    STDMETHODIMP OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame);
    STDMETHODIMP OnNotify(IFilterEvent::EventType type,HRESULT hr,IInputPin* pPinIn,IOutputPin* pPinOut,IMediaFrame* pFrame);
    //IMuxer
    STDMETHODIMP Load(const char* pUrl);
    STDMETHODIMP_(IInputPin*) CreatePin(IMediaType* pMT);
    STDMETHODIMP_(void) Clear();
    //CSdlRender
    static HRESULT UrlSupportQuery(const char* pProtocol,const char* pFormat);
protected:
    string m_name;
    IFilterGraphEvent* m_pFG;
    StreamSet m_streams;
    dom_ptr<Interface> m_spProfile;
    void* m_tag;
    bool m_open;
    int64_t m_expend_clock;
    int64_t m_expend_begin;
    int64_t m_expend_length;
};

#endif // SDLRENDER_H
