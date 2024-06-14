// The functions contained in this file are pretty dummy
// and are included only as a placeholder. Nevertheless,
// they *will* get included in the shared library if you
// don't remove them :)
//
// Obviously, you 'll have to write yourself the super-duper
// functions to include in the resulting library...
// Also, it's not necessary to write every function in this file.
// Feel free to add more files in this project. They will be
// included in the resulting library.
#include <stdio.h>
#include <interface_imp.h>
#include "Library.h"
#include "MemProfile.h"
#include "EventPoint.h"

ISite* g_pSite;
__attribute__((destructor)) void LibraryClose()
{
    if(NULL != g_pSite)
        g_pSite->Release();
        //delete dynamic_cast<CLibrary*>(g_pSite);
}

ISite* GetSite()
{
    if(NULL == g_pSite)
    {
        ISite* pSite = static_cast<ISite*>(CLibrary::Create(IID(ISite)));
        DOM_CLASS_EXPORT(CMemProfile,NULL,0,"profile",MAKE_VERSION(0,0,0,0),NULL,NULL)
        DOM_CLASS_EXPORT(CEventPoint,NULL,0,"event point",MAKE_VERSION(0,0,0,0),NULL,NULL)
        g_pSite = pSite;
    }
    return g_pSite;
}

string guid_to_string(const GUID& guid)
{
	char str[40] = {0};
    snprintf(str,40,"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid.Data1,guid.Data2,guid.Data3,
        guid.Data4[0],guid.Data4[1],guid.Data4[2],guid.Data4[3],
        guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]);
	return str;
}
