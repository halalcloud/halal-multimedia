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
        g_pSite = static_cast<ISite*>(CLibrary::Create(IID(ISite)));
    }
    return g_pSite;
}

string guid_to_string(const GUID* pGuid)
{
	char str[40] = {0};
	if(NULL != pGuid)
	{
        snprintf(str,40,"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            pGuid->Data1,pGuid->Data2,pGuid->Data3,
            pGuid->Data4[0],pGuid->Data4[1],pGuid->Data4[2],pGuid->Data4[3],
            pGuid->Data4[4],pGuid->Data4[5],pGuid->Data4[6],pGuid->Data4[7]);
	}
	return str;
}
