#include "MemProfile.h"
#include <malloc.h>
CMemProfile::CValue::CValue(CMemProfile* pProfile)
:m_pProfile(pProfile)
{
    memset(&m_val,0,sizeof(val));
}

CMemProfile::CValue::~CValue()
{
    Clear();
}

IProfile::val* CMemProfile::CValue::Write(Type type,Value value,Len len)
{
    if(IID_CMP(IID(IProfile),type))
    {
        dom_ptr<IProfile> spProfile;
        JCHK(spProfile.Create(CLSID_CMemProfile,(IProfile*)m_pProfile),NULL);
        m_val.value = spProfile.p;
        spProfile.p = NULL;
    }
    else
    {
        JCHK(NULL != (m_val.value = malloc(len)),NULL);
        if(NULL != value)
            memcpy(m_val.value,value,len);
    }
    m_val.len = len;

    JCHK(m_val.type = (IProfile::Type)realloc((void*)m_val.type,strlen(type)+1),NULL);
    strcpy((char*)m_val.type,type);
    return &m_val;
}

IProfile::val* CMemProfile::CValue::Read(Type type,Value value,Len len)
{
    if(NULL != type)
    {
        JCHK(0 == strcmp(type,m_val.type),NULL);
    }
    if(NULL != value && 0 < len)
    {
        if(len > m_val.len)
            len = m_val.len;
        memcpy(value,m_val.value,len);
    }
    return &m_val;
}

void CMemProfile::CValue::Clear()
{
    if(NULL != m_val.value)
    {
        if(IID_CMP(IID(IProfile),m_val.type))
        {
            ((IProfile*)m_val.value)->Release();
        }
        else
        {
            free(m_val.value);
        }
        m_val.value = NULL;
    }
    if(0 != m_val.len)
        m_val.len = 0;
    if(NULL != m_val.type)
    {
        free((void*)m_val.type);
        m_val.type = NULL;
    }
}

HRESULT CMemProfile::CValue::Load(IStream* pStream)
{
    HRESULT hr;
    uint32_t szStr  = 0;
    JIF(pStream->Read(&szStr,sizeof(szStr)));
    JCHK(m_val.type = (IProfile::Type)realloc((void*)m_val.type,szStr),E_OUTOFMEMORY);
    JIF(pStream->Read((void*)m_val.type,szStr));
    if(IID_CMP(IID(IProfile),m_val.type))
    {
        dom_ptr<IProfile> spProfile;
        JCHK(spProfile.Create(CLSID_CMemProfile,(IProfile*)m_pProfile),E_FAIL);
        m_val.value = spProfile.p;
        dom_ptr<ISerialize> spSerialize;
        JCHK(spProfile.Query(&spSerialize),E_FAIL);
        spProfile.p = NULL;
        JIF(spSerialize->Load(pStream));
    }
    else
    {
        JIF(pStream->Read(&m_val.len,sizeof(m_val.len)));
        JCHK(m_val.value = realloc(m_val.value,m_val.len),E_OUTOFMEMORY);
        JIF(pStream->Read(m_val.value,m_val.len));
    }
    return hr;
}

HRESULT CMemProfile::CValue::Save(IStream* pStream)
{
    HRESULT hr;
    JCHK(NULL != pStream,E_INVALIDARG);
    uint32_t szStr  = strlen(m_val.type) + 1;
    JIF(pStream->Write(&szStr,sizeof(szStr)));
    JIF(pStream->Write((void*)m_val.type,szStr));
    if(IID_CMP(IID(IProfile),m_val.type))
    {
        dom_ptr<IProfile> spProfile((IProfile*)m_val.value);
        dom_ptr<ISerialize> spSerialize;
        JCHK(spProfile.Query(&spSerialize),E_FAIL);
        JIF(spSerialize->Load(pStream));
    }
    else
    {
        JIF(pStream->Write(&m_val.len,sizeof(m_val.len)));
        JIF(pStream->Write(m_val.value,m_val.len));
    }
    return hr;
}

CMemProfile::CMemProfile()
:m_pParent(NULL)
{
    m_it = m_values.end();
}

bool CMemProfile::FinalConstruct(Interface* pOuter,void* pParam)
{
    if(NULL != pOuter)
        m_pParent = static_cast<IProfile*>(pOuter);
    else
        m_pParent = NULL;
    return true;
}

bool CMemProfile::FinalDestructor(bool finally)
{
    if(true == finally)
        Clear();
    return finally;
}

DOM_QUERY_IMPLEMENT_BEG(CMemProfile)
DOM_QUERY_IMPLEMENT(IProfile)
DOM_QUERY_IMPLEMENT(ISerialize)
DOM_QUERY_IMPLEMENT_END

//IProfile
STDMETHODIMP_(IProfile*) CMemProfile::GetParent()
{
    return m_pParent;
}

STDMETHODIMP_(IProfile::val*) CMemProfile::Write(Name name,Type type,Value value,Len len)
{
    JCHK(NULL != name,NULL);
    JCHK(NULL != type,NULL);

    CValue* pVal;
    CValue::It it = m_values.find(name);
    if(it != m_values.end())
    {
        JCHK(NULL != (pVal = it->second),NULL);
    }
    else
    {
        JCHK(NULL != (pVal = new CValue(this)),NULL);
        m_values.insert(CValue::Pair(name,pVal));
    }

    return pVal->Write(type,value,len);
}

STDMETHODIMP_(IProfile::val*) CMemProfile::Read(Name name,Type type,Value value,Len len)
{
    JCHK(NULL != name,NULL);

    CValue* pVal;
    CValue::It it = m_it;
    if(it == m_values.end() || it->first != name)
    {
        it = m_values.find(name);
        if(it == m_values.end())
            return NULL;
    }

    if(NULL == (pVal = it->second))
        return NULL;

    return pVal->Read(type,value,len);
}

STDMETHODIMP_(IProfile::Name) CMemProfile::Next(Name name)
{
    if(NULL == name)
    {
        if(m_it != m_values.end())
            m_it = m_values.end();
    }
    else
    {
        if(m_it == m_values.end() || m_it->first != name)
            m_it = m_values.find(name);
    }
    if(m_it == m_values.end())
        m_it = m_values.begin();
    else
        ++m_it;

    return m_it == m_values.end() ? NULL : m_it->first.c_str();

}

STDMETHODIMP_(IProfile::Name) CMemProfile::Erase(Name name)
{
    if(NULL == name)
    {
        if(m_it != m_values.begin())
            m_it = m_values.begin();
    }
    else
    {
        CValue* pVal;
        CValue::It it = m_it;

        if(it == m_values.end() || it->first != name)
        {
            it = m_values.find(name);
            if(it == m_values.end())
                return NULL;
        }

        if(NULL != (pVal = it->second))
        {
            delete pVal;
            it->second = NULL;
        }
        ++m_it;
        m_values.erase(it);
    }

    return m_it == m_values.end() ? NULL : m_it->first.c_str();
}

STDMETHODIMP_(void) CMemProfile::Clear()
{
    Name name = NULL;
    while(NULL != (name = Erase(name))){}
    m_it = m_values.end();
}

STDMETHODIMP CMemProfile::CopyFrom(IProfile* pProfile,uint32_t flag)
{
    JCHK(NULL != pProfile,E_INVALIDARG);
    return  Copy(this,pProfile,flag);
}

STDMETHODIMP CMemProfile::CopyTo(IProfile* pProfile,uint32_t flag)
{
    JCHK(NULL != pProfile,E_INVALIDARG);
    return  Copy(pProfile,this,flag);
}

STDMETHODIMP CMemProfile::Compare(IProfile* pProfile)
{
    JCHK(NULL != pProfile,E_INVALIDARG);
    return Compare(this,pProfile);
}

STDMETHODIMP CMemProfile::Load(IStream* pStream)
{
    JCHK(NULL != pStream,E_INVALIDARG);
    HRESULT hr;
    uint32_t count = 0;
    JIF(pStream->Read(&count,sizeof(count)));
    while(0 < count--)
    {
        CValue* pVal;
        uint32_t szStr;
        JIF(pStream->Read(&szStr,sizeof(szStr)));

        string name(szStr,0);
        JIF(pStream->Read((void*)name.data(),szStr));
        JCHK(NULL != (pVal = new CValue(this)),E_OUTOFMEMORY);
        m_values.insert(CValue::Pair(name,pVal));
        JIF(pVal->Load(pStream));

    }
    return hr;
}

STDMETHODIMP CMemProfile::Save(IStream* pStream)
{
    HRESULT hr;
    JCHK(NULL != pStream,E_INVALIDARG);
    uint32_t count = m_values.size();
    JIF(pStream->Write(&count,sizeof(count)));
    for(CValue::It it = m_values.begin() ; it != m_values.end() ; ++it)
    {
        uint32_t szStr = it->first.size();
        JIF(pStream->Write(&szStr,sizeof(szStr)));
        JIF(pStream->Write((void*)it->first.data(),szStr));
        JIF(it->second->Save(pStream));
    }
    return hr;
}

HRESULT CMemProfile::Copy(IProfile* pDest,IProfile* pSour,uint32_t flag)
{
    if(pDest == pSour)
        return S_OK;
    HRESULT hr = S_OK;
    Name key = NULL;
    if(0 == flag)
        pDest->Clear();
    while(NULL != (key = pSour->Next(key)))
    {
        const val* pValSour;
        const val* pValDest = pDest->Read(key);
        JCHK(NULL != (pValSour = pSour->Read(key)),E_FAIL);
        if(0 == flag || (NULL != pValDest && 0 != (flag & COPY_FLAG_INTERSECTION)) ||
            (NULL == pValDest && (0 != (flag & COPY_FLAG_COMPILATIONS))))
        {
            JCHK(NULL != (pValDest = pDest->Write(key,pValSour->type,pValSour->value,pValSour->len)),E_FAIL);
            if(IID_CMP(IID(IProfile),pValSour->type))
            {
                IProfile* pSourProfile;
                IProfile* pDestProfile;
                JCHK(NULL != (pSourProfile = (IProfile*)pValSour->value),E_FAIL);
                JCHK(NULL != (pDestProfile = (IProfile*)pValDest->value),E_FAIL);
                JIF(Copy(pDestProfile,pSourProfile,flag));
            }
        }
    }
    return hr;
}

HRESULT CMemProfile::Compare(IProfile* pDest,IProfile* pSour)
{
    if(pDest == pSour)
        return S_OK;
    HRESULT hr = S_OK;
    Name key = NULL;
    while(NULL != (key = pDest->Next(key)))
    {
        const val* pValDest;
        JCHK(pValDest = pDest->Read(key),E_FAIL);
        const val* pValSour = pSour->Read(key);
        if(NULL == pValSour || false == IID_CMP(pValDest->type,pValSour->type))
            return S_FALSE;
        if(IID_CMP(IID(IProfile),pValDest->type))
        {
            IProfile* pSourProfile;
            IProfile* pDestProfile;
            JCHK(NULL != (pSourProfile = (IProfile*)pValSour->value),E_FAIL);
            JCHK(NULL != (pDestProfile = (IProfile*)pValDest->value),E_FAIL);
            JIF(Compare(pDestProfile,pSourProfile));
            if(S_FALSE == hr)
                return hr;
        }
        else
        {
            if(pValDest->len != pValSour->len || (pValDest->value != pValSour->value &&
                (NULL == pValDest->value || NULL == pValSour->value ||
                0 != memcmp(pValDest->value,pValSour->value,pValDest->len))))
                return S_FALSE;
        }
    }
    return hr;
}
