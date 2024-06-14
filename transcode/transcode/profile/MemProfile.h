#ifndef MEMPROFILE_H
#define MEMPROFILE_H
#include <string>
#include <map>
#include <dom.h>
#include <iProfile.h>
#include <iStream.h>

using namespace std;

class CMemProfile : public IProfile , public ISerialize
{
    class CValue
    {
        friend class CMemProfile;

        typedef map<string,CValue*> Collection;
        typedef Collection::iterator It;
        typedef pair<Collection::key_type,Collection::mapped_type> Pair;

        CValue(CMemProfile* pProfile);
        ~CValue();

        val* Write(Type type,Value value,Len len);
        val* Read(Type type,Value value,Len len);
        void Clear();

        HRESULT Load(IStream* pStream);
        HRESULT Save(IStream* pStream);

        CMemProfile* m_pProfile;
        val m_val;
    };
    public:
        DOM_DECLARE(CMemProfile)
        //IProfile
        STDMETHODIMP_(IProfile*) GetParent();
        STDMETHODIMP_(val*) Write(Name name,Type type,Value value,Len len);
        STDMETHODIMP_(val*) Read(Name name,Type type,Value value,Len len);

        STDMETHODIMP_(Value) Write(Name name,const Value value,Len len,Type type);
        STDMETHODIMP_(Value) Read(Name name,Value value,Len len,Type* pType,Len* pLen);
        STDMETHODIMP_(Name) Next(Name name);
        STDMETHODIMP_(Name) Erase(Name name);
        STDMETHODIMP_(void) Clear();
        STDMETHODIMP CopyFrom(IProfile* pProfile,uint32_t flag);
        STDMETHODIMP CopyTo(IProfile* pProfile,uint32_t flag);
        STDMETHODIMP Compare(IProfile* pProfile);
        //ISerialize
        STDMETHODIMP Load(IStream* pStream);
        STDMETHODIMP Save(IStream* pStream);
        //CMemProfile
        static HRESULT Copy(IProfile* pDest,IProfile* pSour,uint32_t flag);
        static HRESULT Compare(IProfile* pDest,IProfile* pSour);
    protected:
        IProfile* m_pParent;
        CValue::Collection m_values;
        CValue::It m_it;
    private:
};

#endif // MEMPROFILE_H
