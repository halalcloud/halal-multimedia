#ifndef IGRAPHBUILDER_INCLUDED
#define IGRAPHBUILDER_INCLUDED
#include <interface.h>
using namespace std;

// {037DCF94-BE66-4b1f-9579-92F4985BE517}
static const CLSID CLSID_CGraphBuilder =
{ 0x37dcf94, 0xbe66, 0x4b1f, { 0x95, 0x79, 0x92, 0xf4, 0x98, 0x5b, 0xe5, 0x17 } };

struct IGraphBuilderCallback
{
    STDMETHOD(OnEnd)(
        HRESULT hr,
        const char* json
        ) PURE;
    STDMETHOD(OnMsg)(
        const char* json
        ) PURE;
};

INTERFACE(IGraphBuilder)
{
    enum info_type
    {
        it_input = 0,
        it_status,
        it_nb
    };
    STDMETHOD(Play)(
        const char* pTask,
        bool isWait = false,
        IGraphBuilderCallback* pCallback = NULL
        ) PURE;
    STDMETHOD(Stop)(
        ) PURE;
    STDMETHOD_(bool,IsRunning)(
        ) PURE;
    STDMETHOD_(const char*,GetInfo)(
        info_type it
        ) PURE;
};


#endif // IGRAPHBUILDER_INCLUDED
