#ifndef MEDIAFRAME_H
#define MEDIAFRAME_H
#include "stdafx.h"

class CMediaFrameAllocate;
class CMediaFrame : public IMediaFrame , public IStream , public ISerialize
{
    typedef list< CMediaFrame* > Set;
    typedef Set::iterator It;
    friend class CMediaFrameAllocate;

    struct Buf
    {
        uint8_t* m_data;
        uint32_t m_size;
        uint8_t  m_flag;

        Buf():m_data(NULL),m_size(0),m_flag(0){}
        ~Buf(){Set(0);}

        HRESULT Set(uint32_t size,const void* data = NULL,uint8_t flag = 0)
        {
            if(0 == size)
            {
                if(NULL != m_data && 0 == (m_flag&IStream::WRITE_FLAG_REFFER))
                {
                    free(m_data);
                }
                m_data = NULL;
                m_size = 0;
                m_flag = 0;
            }
            else
            {
                if(NULL == data || 0 == (flag&IStream::WRITE_FLAG_REFFER))
                {
                    if(0 == (m_flag&IStream::WRITE_FLAG_REFFER))
                    {
                        if(size != m_size)
                        {
                            JCHK(m_data = (uint8_t*)realloc(m_data,size),E_OUTOFMEMORY);
                            m_size = size;
                        }
                    }
                    else
                    {
                        JCHK(m_data = (uint8_t*)malloc(size),E_OUTOFMEMORY);
                        if(size != m_size)
                            m_size = size;
                    }
                    if(NULL != data)
                        memcpy(m_data,data,size);
                }
                else
                {
                    if(NULL != m_data && 0 == (m_flag&IStream::WRITE_FLAG_REFFER))
                        free(m_data);
                    m_data = (uint8_t*)data;
                    m_size = size;
                }
                m_flag = flag;
            }
            return S_OK;
        }
    };

    typedef vector< shared_ptr<Buf> > BufSet;
    typedef BufSet::iterator BufIt;
    typedef vector< buf > bufSet;
    typedef bufSet::iterator bufIt;
    typedef list< dom_ptr<Interface> > ObjSet;
    typedef ObjSet::iterator ObjIt;
    public:
        DOM_DECLARE(CMediaFrame)
        //IMediaFrame
        STDMETHODIMP SetBuf(uint32_t index,uint32_t size,const void* data,uint8_t flag);
        STDMETHODIMP SetBufs(const buf* buf,uint32_t size,uint32_t index,uint8_t flag);
        STDMETHODIMP_(const buf*) GetBuf(uint32_t index,uint32_t* count,uint32_t* size);
        STDMETHODIMP CopyTo(IMediaFrame* pFrame,uint32_t flag);
        STDMETHODIMP CopyFrom(IMediaFrame* pFrame,uint32_t flag);
        STDMETHODIMP_(void) Clear();
        //IStream
        STDMETHODIMP_(uint32_t) GetFlag();
        STDMETHODIMP Open(const char* pUrl,uint32_t mode);
        STDMETHODIMP_(void) Close();
        STDMETHODIMP SetEventEnable(bool enable);
        STDMETHODIMP_(bool) GetEventEnable();
        STDMETHODIMP SetTimer(int id,uint32_t duration,bool one_shoot);
        STDMETHODIMP_(bool) CanRead();
        STDMETHODIMP_(bool) CanWrite();
        STDMETHODIMP Read(void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf);
        STDMETHODIMP Write(const void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf);
        STDMETHODIMP Flush();
        STDMETHODIMP_(int64_t) GetPos();
        STDMETHODIMP_(int64_t) Seek(int64_t position,seek_type type);
        STDMETHODIMP_(int64_t) GetLength();
        STDMETHODIMP SetLength(int64_t len);
        STDMETHODIMP_(bool) IsOpen();
        STDMETHODIMP_(IStream::status&) GetStatus();
        //ISerialize
        STDMETHODIMP Load(IStream* pStream,uint8_t flag,void* param);
        STDMETHODIMP Save(IStream* pStream,uint8_t flag,void* param);
        //CMediaFrame
        HRESULT InternalRead(void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf);
        IMediaFrame* Uses();
        static HRESULT Copy(IMediaFrame* pDest,IMediaFrame* pSour,uint32_t flag);
    protected:
        CMediaFrameAllocate* m_pAllocate;
        It m_it;
        BufSet m_Bufs;
        bufSet m_bufs;
        ObjSet m_objs;
        uint32_t m_idx_read;
        uint32_t m_buf_read;
        uint32_t m_pos_read;
        status m_status;
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
        CMediaFrame::Set m_uses;
        CMediaFrame::Set m_free;
        CLocker  m_lock;
};
#endif // MEDIAFRAME_H
