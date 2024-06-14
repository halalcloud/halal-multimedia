#ifndef SDLRENDER_H
#define SDLRENDER_H
#include "stdafx.h"

class CSdlRender : public IFilter , public ILoad
{
    class CStream
    {
        friend class CSdlRender;
    public:
        CStream(CSdlRender* pRender);
        virtual ~CStream();
        HRESULT Init(IMediaType* pMT,size_t index);
        HRESULT Check(IMediaType* pMT);
        HRESULT Open();
        HRESULT Write(IMediaFrame* pFrame);
        HRESULT Close();
        bool IsOpen();
    protected:
        CSdlRender* m_pRender;
        dom_ptr<IInputPin> m_spPin;
        dom_ptr<IMediaType> m_spMT;
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
    typedef vector< shared_ptr<CStream> > StreamSet;
    typedef StreamSet::iterator StreamIt;
public:
    DOM_DECLARE(CSdlRender)
    //IFilter
    STDMETHODIMP_(FilterType) GetType();
    STDMETHODIMP SetName(const char* pName);
    STDMETHODIMP_(const char*) GetName();
    STDMETHODIMP_(uint32_t) GetFlag();
    STDMETHODIMP_(uint32_t) GetInputPinCount();
    STDMETHODIMP_(IInputPin*) GetInputPin(uint32_t index);
    STDMETHODIMP_(uint32_t) GetOutputPinCount();
    STDMETHODIMP_(IOutputPin*) GetOutputPin(uint32_t index);
    STDMETHODIMP_(IInputPin*) CreateInputPin(IMediaType* pMT);
    STDMETHODIMP_(IOutputPin*) CreateOutputPin(IMediaType* pMT);
    STDMETHODIMP Notify(uint32_t cmd);
    STDMETHODIMP_(uint32_t) GetStatus();
    STDMETHODIMP_(void) SetTag(void* pTag);
    STDMETHODIMP_(void*) GetTag();
    STDMETHODIMP_(double) GetExpend();
    STDMETHODIMP OnConnect(IInputPin* pPin,IOutputPin* pPinOut,IMediaType* pMT);
    STDMETHODIMP OnConnect(IOutputPin* pPin,IInputPin* pPinIn,IMediaType* pMT);
    STDMETHODIMP OnWriteFrame(IInputPin* pPin,IMediaFrame* pFrame);
    //ILoad
    STDMETHODIMP Load(const char* pUrl,uint32_t mode,uint8_t flag = 0);
    //CSdlRender
    HRESULT Open();
    HRESULT Close();
    static HRESULT FilterQuery(const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut);
protected:
    string m_name;
    StreamSet m_streams;
    dom_ptr<IEventPoint> m_ep;
    dom_ptr<IProfile> m_spProfile;
    void* m_pTag;
    IFilter::Status m_status;
    bool m_open;
    int64_t m_expend_clock;
    int64_t m_expend_begin;
    int64_t m_expend_length;
};

#endif // SDLRENDER_H
