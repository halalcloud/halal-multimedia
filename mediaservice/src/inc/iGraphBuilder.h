#ifndef IGRAPHBUILDER_INCLUDED
#define IGRAPHBUILDER_INCLUDED
#include <iMulitimedia.h>
using namespace std;

// {037DCF94-BE66-4b1f-9579-92F4985BE517}
static const CLSID CLSID_CGraphBuilder =
{ 0x37dcf94, 0xbe66, 0x4b1f, { 0x95, 0x79, 0x92, 0xf4, 0x98, 0x5b, 0xe5, 0x17 } };

INTERFACE(IGraphBuilder)
{
    STDMETHOD_(const char*,GetName)(
        ) PURE;
    STDMETHOD(Set)(
        IFilter* pFilter
        ) PURE;
    STDMETHOD(Load)(
        const char* pTask
        ) PURE;
    STDMETHOD_(void,Clear)(
        ) PURE;
    STDMETHOD(PublishPointAddClient)(
        IOutputPin* pPinOut,
        IInputPin* pPinIn
        ) PURE;
};


#endif // IGRAPHBUILDER_INCLUDED
