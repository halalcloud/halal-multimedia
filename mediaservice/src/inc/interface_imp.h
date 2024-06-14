#ifndef INTERFACE_IMP_H
#define INTERFACE_IMP_H
#include <pthread.h>
#include "dom_site.h"

template <class T> class interface_imp : public T
{
public:
    interface_imp(Interface* pOuter,bool aggregate,void* pParam,bool& br)
    :m_pOuter(pOuter)
    ,m_aggregate(aggregate)
    ,m_ref(0)
    {
        if(false == (br = T::FinalConstruct(pOuter,pParam)))
            delete this;
    }
    virtual ~interface_imp()
    {
    }
    //Interface
    STDMETHODIMP_(const ClassInfo&) Class()
    {
        return s_info;
    }
    STDMETHODIMP_(void*) Query(IID iid,bool internal)
    {
        if(NULL != m_pOuter && true == m_aggregate && false == internal)
            return m_pOuter->Query(iid,true);
        else
        {
            void* pResult;
            if(NULL != (pResult = InternalQuery(iid)))
                AddRef(internal);
            return pResult;
        }
    }
    STDMETHODIMP_(REFTYPE) AddRef()
    {
        return AddRef(false);
    }
    STDMETHODIMP_(REFTYPE) Release()
    {
        if(0 == m_ref)
        {
            T::FinalDestructor(true);
            delete this;
            return 0;
        }
        else
        {
            bool is_free = false;
            Interface* pOuter = m_pOuter;
            if(0 == __sync_sub_and_fetch(&m_ref,1))
            {
                if(true == T::FinalDestructor(NULL == m_pOuter))
                {
                    delete this;
                    is_free = true;
                }
            }
            if(NULL == pOuter)
                return false == is_free ? m_ref : 0;
            else
                return pOuter->Release();
        }
    }
    STDMETHODIMP_(Interface*) GetOwner()
    {
        return m_pOuter;
    }
    //interface_imp
    STDMETHODIMP_(void*) QueryInterface(IID iid)
    {
        return NULL == m_pOuter ? Query(iid,false) : InternalQuery(iid);
    }
protected:
    REFTYPE AddRef(bool internal)
    {
        __sync_add_and_fetch(&m_ref,1);
        return false == internal && NULL != m_pOuter ? m_pOuter->AddRef() : m_ref;
    }
    void* InternalQuery(IID iid)
    {
        void* pResult;
        if(IID_NULL == iid || IID_CMP(iid,IID(Interface)))
            pResult = this;
        else
            pResult = T::Query(iid,true);
        return pResult;
    }
private:
    Interface* m_pOuter;
    bool m_aggregate;
    REFTYPE m_ref;
public:
    static ClassInfo s_info;
};

template <class T>
ClassInfo interface_imp<T>::s_info;

#endif // INTERFACE_IMP_H
