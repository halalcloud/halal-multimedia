#ifndef DOM_H
#define DOM_H
#include <typeinfo>
#include <stdio.h>
#include <string.h>
#include "dom_global.h"
#include <interface_imp.h>
/*
#ifdef DOM_EXPORTS
#define DOM_API_(type) extern "C" __attribute__ ((visibility("default"))) type
#else
#define DOM_API_(type) extern "C" __attribute__ ((visibility("default"))) type
#endif
*/
INTERFACE(ITest)
{
    STDMETHOD(Test)(
        )PURE;
};

class CTest : public ITest
{
public:
    CTest(){}
    ~CTest(){}
    HRESULT FinalConstruct(Interface* pThis)
    {
        return 0;
    }
    STDMETHODIMP_(Interface*) Query(IID iid)
    {
        if(IID_TYPE_CMP(iid,ITest))
            return dynamic_cast<ITest*>(this);
        return NULL;
    }
    STDMETHODIMP Test()
    {
        printf("Hello world!\n");
        return 0;
    }
};

void test();

#endif // DOM_H
