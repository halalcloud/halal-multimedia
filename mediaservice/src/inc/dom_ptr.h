#ifndef DOM_PTR_H
#define DOM_PTR_H
#include "dom_site.h"
#include "interface_imp.h"
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

    bool Create(REFCLSID class_id,Interface* pOuter = NULL,bool aggregate = false,void* pParam = NULL)
    {
        void* pResult = g_pSite->CreateObj(class_id,IID(T),pOuter,aggregate,pParam);
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
        JIF(pStream->Write((void*)info.clsid,sizeof(CLSID)));
        JIF(pStream->Write((void*)&info.version,sizeof(info.version)));

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
        void* pResult = obj->Query(IID(T),false);
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

template <class It> class TIt : public IIt , public IEventCallback
{
public:
	TIt():m_set(NULL),m_obj(NULL){}
	virtual ~TIt(){}
    bool FinalConstruct(void* pOuter,void* pParam)
    {
        JCHK(NULL != (m_set = (ISet*)pOuter),false);
        if(NULL != pParam)
            m_it = *(It*)pParam;
        return true;
    }
    bool FinalDestructor(bool is_active)
    {
        return true;
    }
    static HRESULT Create(IIt** ppIt,ISet* pSet,It it)
    {
        JCHK(NULL != ppIt,E_INVALIDARG);
        JCHK(NULL != pSet,E_INVALIDARG);

        TIt<It> *pIt = NULL;
        bool br = false;
        JCHK(pIt = new interface_imp< TIt<It> >(pSet,false,&it,br),E_OUTOFMEMORY);
        if(false == br)
            return E_FAIL;
        else
        {
            JCHK(*ppIt = (TIt*)pIt->Query(IID(IIt),false),E_FAIL);
            return S_OK;
        }
    }
    STDMETHODIMP_(void*)Query(IID iid,bool internal = false)
    {
        if(true == IID_CMP(iid,IID(IIt)))
            return dynamic_cast<IIt*>(this);
        else if(true == IID_CMP(iid,IID(IEventCallback)))
            return dynamic_cast<IEventCallback*>(this);
        else
            return NULL;
    }
    STDMETHODIMP Set(void* it)
    {
        JCHK(NULL != it,E_INVALIDARG);
        m_it = *(It*)it;
        return S_OK;
    }
    STDMETHODIMP Get(void* it)
    {
        JCHK(NULL != it,E_INVALIDARG);
        *(It*)it = m_it;
        return S_OK;
    }
    STDMETHODIMP_(Interface*) Get()
    {
        return m_obj;
    }
    STDMETHODIMP_(bool) Next()
    {
        return m_set->Next(&m_it);
    }
    STDMETHODIMP_(bool) Erase()
    {
        if(m_obj != NULL)
            m_obj = NULL;
        return m_set->Erase(&m_it);
    }
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
    {
        return m_obj == NULL ? E_EOF : m_set->OnEvent(m_obj,this,type,param1,param2,param3);
    }
    STDMETHODIMP_(void) Set(Interface* obj,bool erase = true)
    {
        if(NULL == (m_obj = obj))
        {
            if(true == erase)
                Erase();
        }
    }
protected:
    It m_it;
    ISet* m_set;
    Interface* m_obj;
};

template <class Set,class It> class TSet : public ISet
{
public:
	TSet():m_callback(NULL){}
	virtual ~TSet(){}
    bool FinalConstruct(void* pOuter,void* pParam)
    {
        m_callback = (ICallback*)pParam;
        return true;
    }
    bool FinalDestructor(bool is_active)
    {
        return true;
    }
    static TSet* Create(Interface* pOuter = NULL,bool aggregate = false,void* pParam = NULL)
    {
        bool br = false;
        return new interface_imp< TSet<Set,It> >(pOuter,aggregate,pParam,br);
    }
    STDMETHODIMP OnEvent(Interface* source,IIt* it,uint32_t type,int32_t param1,void* param2,void* param3)
    {
        return NULL == m_callback ? E_EOF : m_callback->OnEvent(this,it,type,param1,param2,param3);
    }
    STDMETHODIMP_(bool) Erase(bool detch = false)
    {
        if(NULL != m_callback)
            m_callback = NULL;
        return true;
    }
    STDMETHODIMP_(void*)Query(IID iid,bool internal = false)
    {
        if(true == IID_CMP(iid,IID(ISet)))
            return dynamic_cast<ISet*>(this);
        return NULL;
    }
    STDMETHODIMP CreateIt(IIt** ppIt,void* pIt = NULL)
    {
        It it = NULL == pIt ? m_set.end() : *(It*)pIt;
        return TIt<It>::Create(ppIt,this,it);
    }
    STDMETHODIMP_(bool) Next(void* it)
    {
        JCHK(NULL != it,false);
        It& _it = *(It*)it;
        if(_it == m_set.end())
            _it = m_set.begin();
        else
            ++_it;
        return _it == m_set.end() ? false : true;
    }
    STDMETHODIMP_(bool) Erase(void* it)
    {
        JCHK(NULL != it,false);
        It& _it = *(It*)it;
        if(_it == m_set.end())
            return false;
        else
        {
            m_set.erase(_it);
            return true;
        }
    }
    STDMETHODIMP_(bool) Empty()
    {
        return m_set.empty();
    }
    STDMETHODIMP_(void) Clear()
    {
        m_set.clear();
    }
    ICallback* m_callback;
    Set m_set;
};

#define SET(set) TSet<set,set::iterator>

#endif // DOM_PTR_H
