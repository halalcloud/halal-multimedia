#ifndef IPROFILE_H_INCLUDED
#define IPROFILE_H_INCLUDED
#include <interface.h>
// {17942BD2-218F-4fed-9AE2-B06CEE4E9B08}
static const CLSID CLSID_CMemProfile =
{ 0x17942bd2, 0x218f, 0x4fed, { 0x9a, 0xe2, 0xb0, 0x6c, 0xee, 0x4e, 0x9b, 0x8 } };

const char PROFILE_TYPE[] = "Profile";

const uint32_t COPY_FLAG_COMPILATIONS = 1;
const uint32_t COPY_FLAG_INTERSECTION = COPY_FLAG_COMPILATIONS << 1;

INTERFACE(IProfile)
{

    typedef const char* Name;
    typedef const char* Type;
    typedef void* Value;
    typedef size_t Len;

    struct val
    {
        Type type;
        Len len;
        Value value;
    };

    STDMETHOD_(IProfile*,GetParent)(
        )PURE;
    STDMETHOD_(val*,Write)(
        Name name,        //name
        Type type,
        Value value,       //value
        Len len
        )PURE;
    STDMETHOD_(val*,Read)(
        Name name,        //name
        Type type = NULL,
        Value value = NULL,
        Len len = 0
        )PURE;
    STDMETHOD_(Name,Next)(
        Name name        //name
        )PURE;
    STDMETHOD_(Name,Erase)(
        Name name        //name
        )PURE;
    STDMETHOD_(void,Clear)(
        )PURE;
    STDMETHOD(CopyFrom)(
        IProfile* pProfile,
        uint32_t flag = 0
        )PURE;
    STDMETHOD(CopyTo)(
        IProfile* pProfile,
        uint32_t flag = 0
        )PURE;
    STDMETHOD(Compare)(
        IProfile* pProfile
        )PURE;
    //template
    template <class T>
    bool Write(Name name,T& value)
    {
        return NULL != Write(name,typeid(T).name(),&value,sizeof(T));
    }

    template <class T>
    bool Write(Name name,T* value)
    {
        JCHK(NULL != value,E_INVALIDARG);
        return NULL != Write(name,typeid(T).name(),value,sizeof(T));
    }

    template <class T>
    T* Write(Name name,T* value,Len len)
    {
        val* pVal = Write(name,typeid(T*).name(),(Value)value,len*sizeof(T));
        return  NULL == pVal ? NULL : static_cast<T*>(pVal->value);
    }

    template <class T>
    bool Read(Name name,T& value)
    {
        return NULL != Read(name,typeid(T).name(),&value,sizeof(T));
    }

    template <class T>
    bool Read(Name name,T* value)
    {
        JCHK(NULL != value,E_INVALIDARG);
        return NULL != Read(name,typeid(T).name(),value,sizeof(T));
    }

    template <class T>
    const T* Read(Name name,T* value,Len len)
    {
        const val* pVal = Read(name,value,len * sizeof(T));
        return  NULL == pVal ? NULL : static_cast<T*>(pVal->value);
    }
};


#endif // IPROFILE_H_INCLUDED
