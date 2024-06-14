#ifndef DOM_PTR_H
#define DOM_PTR_H
#include "dom_site.h"
#include "iStream.h"
template <class T> class dom_ptr
{
public:
    dom_ptr():p(NULL){}
    dom_ptr(T* lp):p(NULL)
    {
        if(NULL != lp)
        {
            lp->AddRef();
            p = lp;
        }
    }
    dom_ptr(const dom_ptr<T>& lp):p(NULL)
    {
        if(NULL != lp.p)
        {
            lp.p->AddRef();
            p = lp.p;
        }
    }
    template <class Q>
    dom_ptr(Q* p):p(NULL)
    {
        QueryFrom(p);
    }
    ~dom_ptr()
    {
        if(NULL != p)
        {
            p->Release();
            p = NULL;
        }
    }
    operator T*() const throw()
    {
        return p;
    }

    T** operator&() throw()
    {
        return &p;
    }

    T* operator=(T* lp)
    {
        if(lp == p)
            return lp;
        if(NULL != lp)
            lp->AddRef();
        if(NULL != p)
            p->Release();
        return p = lp;
    }

    T* operator=(const dom_ptr<T>& lp)
    {
        if(NULL != lp.p)
            lp.p->AddRef();
        if(NULL != p)
            p->Release();
        return p = lp.p;
    }

    T* operator->() const throw()
    {
        return p;
    }

    bool null()
    {
        return p == NULL;
    }

    bool Create(REFCLSID class_id,Interface* pOuter = NULL,void* pParam = NULL)
    {
        void* pResult = g_pSite->CreateObj(class_id,IID(T),pOuter,pParam);
        if(NULL == pResult)
            return false;
        if(NULL != p)
            p->Release();
        p = static_cast<T*>(pResult);
        return true;
    }

    HRESULT Load(IStream* pStream)
    {
        HRESULT hr;
        JCHK(NULL != pStream,E_INVALIDARG);
        CLSID clsid;
        OBJ_VERSION ver;
        JIF(pStream->Read(&clsid,sizeof(clsid)));
        JIF(pStream->Read(&ver,sizeof(ver)));
        JCHK(Create(clsid),E_FAIL);
        dom_ptr<ISerialize> spSerialize;
        if(true == Query(&spSerialize))
        {
            JIF(spSerialize->Load(pStream));
        }
        return hr;
    }

    HRESULT Save(IStream* pStream)
    {
        HRESULT hr;
        JCHK(NULL != pStream,E_INVALIDARG);
        JCHK(NULL != p,E_FAIL);

        const ClassInfo& info = p->Class();
        JIF(pStream->Write((void*)info.pClsid,sizeof(CLSID)));
        JIF(pStream->Write((void*)&info.ver,sizeof(info.ver)));

        dom_ptr<ISerialize> spSerialize;
        if(true == Query(&spSerialize))
        {
            JIF(spSerialize->Save(pStream));
        }
        return hr;
    }

    template <class Q>
    bool Query(Q** pp) const throw()
    {
        if(NULL == pp || NULL == p)
            return false;
        return NULL == (*pp = static_cast<Q*>(p->Query(typeid(Q*).name()))) ? false : true;
    }

    template <class Q>
    bool QueryFrom(Q* obj)throw()
    {
        if(NULL == obj)
            return false;
        void* pResult = obj->Query(IID(T));
        if(NULL == pResult)
            return false;
        if(NULL != p)
            p->Release();
        p = static_cast<T*>(pResult);
        return true;
    }

    HRESULT CopyTo(T** pp)
    {
        if(NULL == pp)
            return E_INVALIDARG;
        *pp = p;
        if(NULL != p)
            p->AddRef();
        return S_OK;
    }
    T* p;
};


#endif // DOM_PTR_H
