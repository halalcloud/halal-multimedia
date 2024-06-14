#ifndef BUFSESSION_H
#define BUFSESSION_H

#include <list>
#include "stdafx.h"
#include <Locker.h>

class CBufSession : public IBufSession , public IEpollCallback
{
    typedef list< IBuffer* > BufferSet;
    typedef BufferSet::iterator BufferIt;
public:
    DOM_DECLARE(CBufSession)
    //IBufSession
    STDMETHODIMP Open(IStreamServer* pServer);
    STDMETHODIMP Open(url* pUrl,int mode);
    STDMETHODIMP Receive(IBuffer** ppBuf);
    STDMETHODIMP Deliver(IBuffer* pBuf);
    //IEpollCallback
    STDMETHODIMP OnEvent(uint32_t id,void* pParam);
    STDMETHODIMP_(IEpoll*) GetEpoll();
    STDMETHODIMP_(bool) IsOpen();
    //CObjSession
    HRESULT Read(uint8_t* pBuf,uint32_t len,uint32_t& pos);
    HRESULT Write(uint8_t* pBuf,uint32_t len,uint32_t& pos);
    HRESULT Read(IBuffer* pBuf);
    HRESULT Write(IBuffer* pBuf);
protected:
    IEpollCallback* m_callback;
    dom_ptr<IStream> m_spStream;
    BufferSet m_bufs_receive;
    BufferSet m_bufs_deliver;
    dom_ptr<IBuffer> m_buf_receive;
    bool m_ansy;
    bool m_bound;
    uint32_t m_stp_receive;
    uint32_t m_pos_recevie;
    uint32_t m_stp_deliver;
    uint32_t m_pos_deliver;
    CLocker m_locker_read;
    CLocker m_locker_write;
};

#endif // BUFSESSION_H
