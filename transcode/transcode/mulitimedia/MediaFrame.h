#ifndef MEDIAFRAME_H
#define MEDIAFRAME_H
#include <list>
#include <pthread.h>
#include "stdafx.h"
class CMediaFrame;
typedef list< CMediaFrame* > FrameSet;
typedef FrameSet::iterator FrameIt;
class CMediaFrameAllocate;
class CMediaFrame : public IMediaFrame
{
    friend class CMediaFrameAllocate;
    public:
        DOM_DECLARE(CMediaFrame)
        //IMediaFrame
        STDMETHODIMP_(void) Init();
        STDMETHODIMP Alloc(uint32_t size);
        STDMETHODIMP_(uint8_t*) GetBuf(uint32_t* pSize);
        STDMETHODIMP SetBuf(uint8_t* pBuf,uint32_t lenBuf);
        STDMETHODIMP CopyTo(IMediaFrame* pFrame,uint32_t flag);
        STDMETHODIMP CopyFrom(IMediaFrame* pFrame,uint32_t flag);
        STDMETHODIMP_(void) Clear();
        //CMediaFrame
        IMediaFrame* Uses();
        static HRESULT Copy(IMediaFrame* pDest,IMediaFrame* pSour,uint32_t flag);
    protected:
        CMediaFrameAllocate* m_pAllocate;
        FrameIt m_it;
        uint8_t* m_pBuf;
        size_t m_szBuf;
        size_t m_szUse;
};

class CMediaFrameAllocate : public IMediaFrameAllocate
{
    friend class CMediaFrame;
    public:
        DOM_DECLARE(CMediaFrameAllocate)
        STDMETHODIMP Alloc(IMediaFrame** ppFrame);
        STDMETHODIMP_(void) Clear();
    protected:
        void Init(CMediaFrame* pFrame);
        void Uses(CMediaFrame* pFrame);
        bool Free(CMediaFrame* pFrame);
        void Clear(bool isRelease);
    protected:
        FrameSet m_uses;
        FrameSet m_free;
};
#endif // MEDIAFRAME_H
