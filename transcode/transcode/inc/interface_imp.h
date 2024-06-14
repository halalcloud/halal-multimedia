#ifndef INTERFACE_IMP_H
#define INTERFACE_IMP_H
#include <pthread.h>
#include "dom_site.h"

template <class T> class interface_imp : public T
{
    template <class U> class iunknown : public Interface
    {
    public:
        iunknown(interface_imp<U>* pThis,Interface* pOuter)
        :m_pThis(pThis)
        ,m_ref(0)
        ,m_pOuter(pOuter){}
        virtual ~iunknown()
        {
        }
        //Interface
        STDMETHODIMP_(const ClassInfo&) Class()
        {
            return m_pThis->s_info;
        }
        STDMETHODIMP_(void*) Query(IID iid)
        {
            void* pResult;
            if(NULL != (pResult = m_pThis->InternalQuery(iid)))
                AddRef();
            return pResult;
        }
        STDMETHODIMP_(REFTYPE) AddRef()
        {
            return __sync_add_and_fetch(&m_ref,1);
        }
        STDMETHODIMP_(REFTYPE) Release()
        {
            return Release(NULL);
        }
        //iunknown
        REFTYPE Release(Interface** ppOuter)
        {
//            if(STR_CMP(Class().pName,"CFFmpegAudioEncoder"))
//            {
//                int i = 0;
//            }
            if(0 == m_ref)
            {
                if(NULL != ppOuter)
                    *ppOuter = NULL;
                m_pThis->FinalDestructor(true);
                delete m_pThis;
                return 0;
            }
            else
            {
                if(NULL != ppOuter)
                    *ppOuter = m_pOuter;
                if(0 == __sync_sub_and_fetch(&m_ref,1))
                {
                    if(NULL == m_pOuter || true == m_pThis->FinalDestructor(false))
                    {
                        delete m_pThis;
                        return 0;
                    }
                }
                return m_ref;
            }
        }
    protected:
        interface_imp<U>* m_pThis;
        REFTYPE m_ref;
    public:
        Interface* m_pOuter;
    };
public:
    interface_imp(Interface* pOuter,void* pParam,bool& br)
    :m_unk(this,pOuter)
    ,m_this(NULL)
    {
        if(false == (br = T::FinalConstruct(pOuter,pParam)))
            delete this;
    }
    ~interface_imp()
    {
    }
    //Interface
    STDMETHODIMP_(const ClassInfo&) Class()
    {
        return s_info;
    }
    STDMETHODIMP_(void*) Query(IID iid)
    {
        void* pResult;
        if(m_this == &m_unk)
            pResult = m_unk.m_pOuter->Query(iid);
        else
        {
            if(NULL != (pResult = InternalQuery(iid)))
                AddRef();
        }
        return pResult;
    }
    STDMETHODIMP_(REFTYPE) AddRef()
    {
        REFTYPE ref = m_unk.AddRef();
        return NULL == m_unk.m_pOuter ? ref : m_unk.m_pOuter->AddRef();
    }
    STDMETHODIMP_(REFTYPE) Release()
    {
        Interface* pOuter;
        REFTYPE ref = m_unk.Release(&pOuter);
        return NULL == pOuter ? ref : pOuter->Release();
    }
    //Interface
    STDMETHODIMP_(void*) QueryInterface(IID iid)
    {
        return NULL == m_unk.m_pOuter ? Query(iid) : m_this = static_cast<Interface*>(InternalQuery(iid));
    }
protected:
    void* InternalQuery(IID iid)
    {
        void* pResult;
        if(IID_CMP(iid,IID(Interface)))
            pResult = &m_unk;
        else
            pResult = T::Query(iid);
        return pResult;
    }
    bool FinalDestructor(bool finally)
    {
        return T::FinalDestructor(finally);
    }
private:
    iunknown<T> m_unk;
    Interface* m_this;
public:
    static ClassInfo s_info;
};

template <class T>
ClassInfo interface_imp<T>::s_info;
#endif // INTERFACE_IMP_H
