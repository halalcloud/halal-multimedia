#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <list>
#include <memory>
#include <dom.h>
#include <iStream.h>
#include <iProfile.h>
#include <iGraphBuilder.h>
#include "../src/Url.cpp"
#include <transcode.h>
int getch(void) {
    struct termios tm, tm_old;
    int fd = STDIN_FILENO, c;
    if(tcgetattr(fd, &tm) < 0)
    return -1;
    tm_old = tm;
    cfmakeraw(&tm);
    if(tcsetattr(fd, TCSANOW, &tm) < 0)
    return -1;
    c = fgetc(stdin);
    if(tcsetattr(fd, TCSANOW, &tm_old) < 0)
    return -1;
    return c;
}

ISite* g_pSite;

int main(int argc,char *argv[])
{
    if(argc < 2)
    {
        printf("---------------transcode-------------\n");
        printf("Useage:%s [node url/json file]\n",strrchr(argv[0],'/')+1);
        return 0;
    }
    HRESULT hr = S_OK;
    if(NULL == (g_pSite = GetSite()))
        return hr;

    ISite::DUMP_FLAG flag = g_pSite->DumpGetFlag();
    flag |= DOT_ERR_ASSERT | DOT_ERR_PRINTF | DOT_ERR_FILE | DOT_LOG_PRINTF;
    g_pSite->DumpSetFlag(flag);
    g_pSite->Load("./");  //load ./ all plug

    dom_ptr<IEpoll> spEpoll;
    JCHK(spEpoll.Create(CLSID_CEpoll),E_FAIL);
    g_pSite->SetObj(spEpoll);

    dom_ptr<IGraphBuilder> spGB;
    JCHK(spGB.Create(CLSID_CGraphBuilder),E_FAIL);
    ifstream ifs(argv[1]);
    string json((istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
    spGB->Load(json.c_str());
//    while(spGB->IsRunning())
//    {
//        sleep(1);
//        const char* msg = spGB->GetInfo(IGraphBuilder::it_status);
//        if(NULL != msg)
//            printf("status:%s\n",msg);
//    }
    while('\n' != getchar()){}
    return hr;
}
