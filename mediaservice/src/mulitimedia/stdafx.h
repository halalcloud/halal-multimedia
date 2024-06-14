#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED
#include <list>
#include <map>
#include <pthread.h>
#include <memory>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <string>
#include <dom.h>
#include <iMulitimedia.h>
#include <iMediaService.h>
#include <iGraphBuilder.h>
#include <list>
#include <time_expend.h>
#include <Locker.h>
#include "../src/Url.cpp"

using namespace std;
using namespace Poco;
using namespace Poco::JSON;

typedef multimap< HRESULT,const ClassInfo*,greater<HRESULT> > ClassSet;
typedef ClassSet::iterator ClassIt;
typedef pair<ClassSet::key_type,ClassSet::mapped_type> ClassPair;

typedef list< dom_ptr<IMediaFrame> > FrameSet;
typedef FrameSet::iterator FrameIt;

extern string g_config_root;
int64_t GetColockCount();

HRESULT QueryFilter(ClassSet& classes,uint32_t sub = 0,const char* pName = NULL,const char* protocol = NULL,const char* format = NULL,IMediaType* pMtIn = NULL,IMediaType* pMtOut = NULL);
HRESULT Convert(IProfile* pProfile,const Object::Ptr& obj);
HRESULT FrameBufferInput(FrameSet& buf,IMediaFrame* pFrame,int64_t len,uint32_t& count);

#endif // STDAFX_H_INCLUDED
