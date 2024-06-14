#include <stdio.h>
#include <dom.h>
#include <iMediaService.h>
ISite* g_pSite;

int main(int argc,char *argv[])
{
    HRESULT hr = S_OK;
    if(NULL == (g_pSite = GetSite()))
        return hr;

    ISite::DUMP_FLAG flag = g_pSite->DumpGetFlag();
    flag = DOT_ERR_ASSERT | DOT_ERR_PRINTF | DOT_ERR_FILE | DOT_LOG_PRINTF;
    g_pSite->DumpSetFlag(flag);
//    g_pSite->DumpSetLogLevelRange(4,4);
    g_pSite->Load("./");  //load ./ all plug

    dom_ptr<IEpoll> spEpoll;
    JCHK(spEpoll.Create(CLSID_CEpoll),E_FAIL);
    g_pSite->SetObj(spEpoll);

    dom_ptr<IMediaService> spService;
    JCHK(spService.Create(CLSID_CMediaService),E_FAIL);
    JIF(spService->StartUp(1 < argc ? argv[1] : NULL));

    int c;
    while ((c = getchar()) != '\n'){}
    g_pSite->SetObj(NULL);
    spEpoll = NULL;
    JIF(spService->Shutdown());

    return hr;
}
