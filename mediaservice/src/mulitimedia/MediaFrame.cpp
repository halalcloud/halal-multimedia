#include "MediaFrame.h"
#include <malloc.h>

CMediaFrame::CMediaFrame()
:m_pAllocate(NULL)
,m_idx_read(0)
,m_buf_read(0)
,m_pos_read(0)
{
    memset(&info,0,sizeof(info));
    info.dts = MEDIA_FRAME_NONE_TIMESTAMP;
    info.pts = MEDIA_FRAME_NONE_TIMESTAMP;
    memset(&m_status,0,sizeof(m_status));
}

bool CMediaFrame::FinalConstruct(Interface* pOuter,void* pParam)
{
    m_pAllocate = static_cast<CMediaFrameAllocate*>(pOuter);
    if(NULL != m_pAllocate)
        m_pAllocate->Init(this);
    return true;
}

bool CMediaFrame::FinalDestructor(bool finally)
{
    if(false == finally)
    {
        Clear();
        if(NULL != m_pAllocate)
            finally = m_pAllocate->Free(this);
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CMediaFrame)
DOM_QUERY_IMPLEMENT(IMediaFrame)
DOM_QUERY_IMPLEMENT(IStream)
DOM_QUERY_IMPLEMENT(ISerialize)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CMediaFrame::SetBuf(uint32_t index,uint32_t size,const void* data,uint8_t flag)
{
    HRESULT hr;
    if(index >= m_Bufs.size())
    {
        Buf* pBuf;
        JCHK(pBuf = new Buf(),E_OUTOFMEMORY);
        shared_ptr<Buf> Buf(pBuf);

        JIF(pBuf->Set(size,data,flag));
        m_Bufs.push_back(Buf);
        m_bufs.push_back(buf{pBuf->m_data,pBuf->m_size});
        index = m_Bufs.size() - 1;
    }
    else
    {
        shared_ptr<Buf>& Buf = m_Bufs.at(index);
        buf& buf = m_bufs.at(index);
        Buf->Set(size,data,flag);
        buf.data = Buf->m_data;
        buf.size = Buf->m_size;
    }
    return index;
}

STDMETHODIMP CMediaFrame::SetBufs(const buf* buf,uint32_t size,uint32_t index,uint8_t flag)
{
    JCHK(NULL != buf,E_INVALIDARG);
    JCHK(0 < size,E_INVALIDARG);
    if(MAX_MEDIA_FRAME_BUF_COUNT == index)
        index = m_bufs.size();
    else
    {
        JCHK(index <= m_bufs.size(),E_INVALIDARG);
    }

    HRESULT hr = S_OK;
    size += index;
    for(uint32_t i=index ; i<size ; ++i)
    {
        JIF(SetBuf(i,buf->size,buf->data,flag));
        ++buf;
    }
    return hr;
}

STDMETHODIMP_(const IMediaFrame::buf*) CMediaFrame::GetBuf(uint32_t index,uint32_t* count,uint32_t* size)
{
    if(index >= m_bufs.size())
    {
        if(NULL != count)
            *count = 0;
        if(NULL != size)
            *size = 0;
        return NULL;
    }

    const IMediaFrame::buf* pBuf;
    JCHK(pBuf = m_bufs.data(),NULL);
    pBuf += index;

    uint32_t left = m_bufs.size() - index;

    if(NULL != count)
    {
        if(*count > left)
            *count = left;
        else
            left = *count;
    }

    if(NULL != size)
    {
        *size = 0;
        for(uint32_t i=0 ; i < left ; ++i)
            *size += pBuf[i].size;
    }

    return pBuf;
}

STDMETHODIMP CMediaFrame::CopyTo(IMediaFrame* pFrame,uint32_t flag)
{
    return Copy(pFrame,this,flag);
}

STDMETHODIMP CMediaFrame::CopyFrom(IMediaFrame* pFrame,uint32_t flag)
{
    return Copy(this,pFrame,flag);
}

STDMETHODIMP_(void) CMediaFrame::Clear()
{
    m_idx_read = 0;
    m_buf_read = 0;
    m_pos_read = 0;
    m_bufs.clear();
    m_Bufs.clear();
    m_objs.clear();
}

STDMETHODIMP_(uint32_t) CMediaFrame::GetFlag()
{
    return 0;
}

STDMETHODIMP CMediaFrame::Open(const char* pUrl,uint32_t mode)
{
    return S_OK;
}

STDMETHODIMP_(void) CMediaFrame::Close()
{
}

STDMETHODIMP CMediaFrame::SetEventEnable(bool enable)
{
    return E_FAIL;
}

STDMETHODIMP_(bool) CMediaFrame::GetEventEnable()
{
    return false;
}

STDMETHODIMP CMediaFrame::SetTimer(int id,uint32_t duration,bool one_shoot)
{
    return E_FAIL;
}

STDMETHODIMP_(bool) CMediaFrame::CanRead()
{
    return false == m_bufs.empty();
}

STDMETHODIMP_(bool) CMediaFrame::CanWrite()
{
    return 0 == m_pos_read;
}

STDMETHODIMP CMediaFrame::Read(void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf)
{
    JCHK(NULL != pBuf,E_INVALIDARG);
    return InternalRead(pBuf,szBuf,flag,ppBuf);
}

STDMETHODIMP CMediaFrame::Write(const void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf)
{
    HRESULT hr = S_OK;
    if(0 != (flag & IStream::WRITE_FLAG_FRAME))
    {
        JCHK(NULL != pBuf,E_INVALIDARG);
        IMediaFrame* pFrame = (IMediaFrame*)pBuf;
        uint32_t count = MAX_MEDIA_FRAME_BUF_COUNT;
        const IMediaFrame::buf* pBuf;
        if(NULL != (pBuf = pFrame->GetBuf(0,&count)))
        {
            JIF(SetBufs(pBuf,count,MAX_MEDIA_FRAME_BUF_COUNT,flag));
            if(0 != (IStream::WRITE_FLAG_REFFER & flag))
                m_objs.push_back(pFrame);
        }
    }
    else if(0 != (flag & IStream::WRITE_FLAG_INTERFACE))
    {
        m_objs.push_back((Interface*)pBuf);
    }
    else
    {
        JCHK(0 < szBuf,E_INVALIDARG);
        JIF(SetBuf(MAX_MEDIA_FRAME_BUF_COUNT,szBuf,pBuf,flag));
        if(NULL != ppBuf)
            *ppBuf = m_Bufs.back()->m_data;
    }
    return hr;
}

STDMETHODIMP CMediaFrame::Flush()
{
    return E_FAIL;
}

STDMETHODIMP_(int64_t) CMediaFrame::GetPos()
{
    return m_pos_read;
}

STDMETHODIMP_(int64_t) CMediaFrame::Seek(int64_t position,seek_type type)
{
    m_pos_read = 0;
    m_buf_read = 0;
    m_idx_read = 0;
    return InternalRead(NULL,(uint32_t)position,0,NULL);
}

STDMETHODIMP_(int64_t) CMediaFrame::GetLength()
{
    int64_t len = 0;
    for(bufIt it = m_bufs.begin() ; it != m_bufs.end() ; ++it)
    {
        buf& buf = *it;
        len += buf.size;
    }
    return len;
}

STDMETHODIMP CMediaFrame::SetLength(int64_t len)
{
    return E_FAIL;
}

STDMETHODIMP_(bool) CMediaFrame::IsOpen()
{
    return true;
}

STDMETHODIMP_(IStream::status&) CMediaFrame::GetStatus()
{
    return m_status;
}

STDMETHODIMP CMediaFrame::Load(IStream* pStream,uint8_t flag,void* param)
{
    HRESULT hr;

    JCHK(NULL != param,E_INVALIDARG);
    int64_t base = *(int64_t*)param;

    JIF(pStream->Read(&info.flag,sizeof(info.flag)));

    int32_t dts,pts;
    JIF(pStream->Read(&dts,sizeof(dts),flag));
    JIF(pStream->Read(&pts,sizeof(pts),flag));

    info.dts = dts * 10000 + base;
    info.pts = pts * 10000 + base;

    uint32_t size;
    JIF(pStream->Read(&size,sizeof(size)));

    Clear();
    if(0 < size)
    {
        const buf* pBuf;
        JIF(SetBuf(0,size,NULL,0));
        JCHK(pBuf = GetBuf(0,NULL,NULL),E_FAIL);
        JIF(pStream->Read(pBuf->data,pBuf->size,flag));
    }
    return hr;
}

STDMETHODIMP CMediaFrame::Save(IStream* pStream,uint8_t flag,void* param)
{
    HRESULT hr;
    JCHK(NULL != param,E_INVALIDARG);
    int64_t base = *(int64_t*)param;

    JIF(pStream->Write(&info.flag,sizeof(info.flag)));

    int32_t dts = int32_t((info.dts - base)/10000.0 + 0.5);
    int32_t pts = int32_t((info.pts - base)/10000.0 + 0.5);

    JIF(pStream->Write(&dts,sizeof(dts)));
    JIF(pStream->Write(&pts,sizeof(pts)));

    uint32_t size = 0;
    GetBuf(0,NULL,&size);
    JIF(pStream->Write(&size,sizeof(size)));
    if(0 < size)
    {
        JIF(pStream->Write((IMediaFrame*)this,0,IStream::WRITE_FLAG_FRAME|flag));
    }
    return hr;
}

HRESULT CMediaFrame::InternalRead(void* pBuf,uint32_t szBuf,uint8_t flag,void** ppBuf)
{
    if(m_idx_read >= m_bufs.size())
        return E_AGAIN;

    HRESULT hr = 0;
    uint8_t* pData = (uint8_t*)pBuf;
    const IMediaFrame::buf* pBufs = m_bufs.data();
    uint32_t pos = m_buf_read;
    uint32_t i = m_idx_read;
    for( ; i < m_bufs.size() ; ++i)
    {
        uint32_t left = pBufs[i].size - pos;

        if(szBuf < left)
        {
            if(0 < szBuf)
            {
                if(NULL != pData)
                    memcpy(pData,(uint8_t*)pBufs[i].data + pos,szBuf);
                pos += szBuf;
                hr += szBuf;
                szBuf = 0;
            }
            break;
        }
        else
        {
            if(NULL != pData)
                memcpy(pData,(uint8_t*)pBufs[i].data + pos,left);
            szBuf -= left;
            pData += left;
            pos = 0;
            hr += left;
        }
    }

    if(0 < szBuf)
        return E_AGAIN;

    if(0 == (flag & IStream::READ_FLAG_PEEK))
    {
        m_pos_read += hr;
        m_buf_read = pos;
        m_idx_read = i;
    }

    return hr;
}

IMediaFrame* CMediaFrame::Uses()
{
    if(NULL != m_pAllocate)
    {
        m_pAllocate->Uses(this);
        memset(&info,0,sizeof(info));
        info.dts = MEDIA_FRAME_NONE_TIMESTAMP;
        info.pts = MEDIA_FRAME_NONE_TIMESTAMP;
        AddRef();
        return dynamic_cast<IMediaFrame*>(this);
    }
    else
        return NULL;
}

HRESULT CMediaFrame::Copy(IMediaFrame* pDest,IMediaFrame* pSour,uint32_t flag)
{
    JCHK(NULL != pDest,E_INVALIDARG);
    JCHK(NULL != pSour,E_INVALIDARG);
    if(0 != (flag & MEDIAFRAME_COPY_INFO))
    {
        pDest->info = pSour->info;
    }
    uint8_t w_flag = IStream::WRITE_FLAG_FRAME;
    if(0 == (flag & MEDIAFRAME_COPY_DATA))
    {
        w_flag |=  IStream::WRITE_FLAG_REFFER;
    }
    pDest->Clear();
    dom_ptr<IStream> spStream;
    JCHK(spStream.QueryFrom(pDest),E_FAIL);
    return spStream->Write(pSour,0,w_flag);
}

CMediaFrameAllocate::CMediaFrameAllocate()
{
}

bool CMediaFrameAllocate::FinalConstruct(Interface* pOuter,void* pParam)
{
    return true;
}

bool CMediaFrameAllocate::FinalDestructor(bool finally)
{
    if(true == finally)
    {
        Clear(true);
    }
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CMediaFrameAllocate)
DOM_QUERY_IMPLEMENT(IMediaFrameAllocate)
DOM_QUERY_IMPLEMENT_END

STDMETHODIMP CMediaFrameAllocate::Alloc(IMediaFrame** ppFrame)
{
	JCHK(NULL != ppFrame,E_INVALIDARG);
    CLocker lock(m_lock);
    CMediaFrame::It it = m_free.begin();
	if(it == m_free.end())
	{
        IMediaFrame* pFrame;
        JCHK(pFrame = static_cast<IMediaFrame*>(CMediaFrame::Create(IID(IMediaFrame),this)),E_FAIL);
        pFrame->AddRef();
        *ppFrame = pFrame;
	}
	else
	{
        CMediaFrame* pFrame;
        JCHK(pFrame = *it,E_FAIL);
        JCHK(*ppFrame = pFrame->Uses(),E_FAIL);
	}
	return S_OK;
}

STDMETHODIMP_(void) CMediaFrameAllocate::Clear()
{
    Clear(false);
}

void CMediaFrameAllocate::Init(CMediaFrame* pFrame)
{
    pFrame->m_it = m_uses.insert(m_uses.end(),pFrame);
}

void CMediaFrameAllocate::Uses(CMediaFrame* pFrame)
{
    m_free.erase(pFrame->m_it);
    pFrame->m_it = m_uses.insert(m_uses.end(),pFrame);
}

bool CMediaFrameAllocate::Free(CMediaFrame* pFrame)
{
    CLocker lock(m_lock);
    m_uses.erase(pFrame->m_it);
    if(true == m_free.empty())
    {
        pFrame->m_it = m_free.insert(m_free.end(),pFrame);
        return false;
    }
    else
        return true;
}

void CMediaFrameAllocate::Clear(bool isRelease)
{
    CLocker lock(m_lock);
	CMediaFrame* pFrame;
	CMediaFrame::It it = m_free.begin();
	while(it != m_free.end())
	{
		if(NULL != (pFrame = *it))
            pFrame->Release();
		it = m_free.erase(it);
	}
	if(true == isRelease)
	{
        it = m_uses.begin();
        while(it != m_uses.end())
        {
            if(NULL != (pFrame = *it))
                pFrame->Release();
            it = m_uses.erase(it);
        }
	}
}
