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
#include "stdafx.h"
#include "MediaFrame.h"
#include "InputPin.h"
#include "OutputPin.h"
#include "PublishRender.h"
#include "FilterGraph.h"
#include "GraphBuilder.h"
#include "MediaService.h"
#include "FileSession.h"
#include "ApiSession.h"
#include <time_expend.cpp>

string g_config_root = "/root";

DOM_CLASS_EXPORT_BEG
DOM_CLASS_EXPORT(CMediaFrame,NULL,0,"media frame",MAKE_VERSION(0,0,0,0),NULL,NULL)
DOM_CLASS_EXPORT(CMediaFrameAllocate,NULL,0,"media frame allocate",MAKE_VERSION(0,0,0,0),NULL,NULL)
DOM_CLASS_EXPORT(CInputPin,NULL,0,"input pin",MAKE_VERSION(0,0,0,0),NULL,NULL)
DOM_CLASS_EXPORT(COutputPin,NULL,0,"output pin",MAKE_VERSION(0,0,0,0),NULL,NULL)
DOM_CLASS_EXPORT(CPublishRender,FILTER_TYPE_NAME,FT_Render,PROTOCOL_PUBLISH_NAME,MAKE_VERSION(0,0,0,0),(void*)CPublishRender::FilterQuery,NULL)
DOM_CLASS_EXPORT(CFilterGraph,NULL,0,"filter graph",MAKE_VERSION(0,0,0,0),NULL,NULL)
DOM_CLASS_EXPORT(CGraphBuilder,NULL,0,"graph builder",MAKE_VERSION(0,0,0,0),NULL,NULL)
DOM_CLASS_EXPORT(CMediaService,NULL,0,"media service",MAKE_VERSION(0,0,0,0),NULL,NULL)
DOM_CLASS_EXPORT(CFileSession,FILTER_TYPE_NAME,FT_Session,"file session",MAKE_VERSION(0,0,0,0),(void*)CFileSession::FilterQuery,NULL)
DOM_CLASS_EXPORT(CApiSession,FILTER_TYPE_NAME,FT_Session,"api session",MAKE_VERSION(0,0,0,0),(void*)CApiSession::FilterQuery,NULL)
DOM_CLASS_EXPORT_END

HRESULT QueryFilter(ClassSet& classes,uint32_t sub,const char* pName,const char* protocol,const char* format,IMediaType* pMtIn,IMediaType* pMtOut)
{
    HRESULT hr;
    classes.clear();
    dom_ptr<IIt> spIt;
    JIF(g_pSite->EnumClass(&spIt));
    while(spIt->Next())
    {
        const class_info* info;
        JCHK(info = g_pSite->GetClass(spIt),E_FAIL);
        if((true == STR_CMP(info->pInfo->major,FILTER_TYPE_NAME)) &&
            (0 == sub || sub == (sub & info->pInfo->sub)) &&
            (NULL == pName || true == STR_CMP(info->pInfo->name,pName)))
        {
            if(NULL != protocol || NULL != format || NULL != pMtIn || NULL != pMtOut)
            {
                FILTER_QUERY_FUNC* pFunc = (FILTER_QUERY_FUNC*)info->pInfo->ext1;
                if(NULL != pFunc)
                {
                    HRESULT hr = pFunc(protocol,format,pMtIn,pMtOut);
                    if(0 <= hr)
                    {
                        classes.insert(ClassPair(hr,info->pInfo));
                    }
                }
            }
            else
            {
                classes.insert(ClassPair(0,info->pInfo));
            }
        }
    }
    return hr;
}

int64_t GetColockCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    return ts.tv_sec * 10000000 + ts.tv_nsec / 100;
}

HRESULT Convert(IProfile* pProfile,const Object::Ptr& obj)
{
    HRESULT hr = S_OK;
    for(Object::ConstIterator it=obj->begin() ; it!=obj->end() ; ++it)
    {
        if(true == obj->isObject(it))
        {
            Object::Ptr objTemp = it->second.extract<Object::Ptr>();
            if(false == objTemp.isNull())
            {
                IProfile::val* pVal = pProfile->Write(it->first.c_str(),IID(IProfile),NULL,0);
                if(NULL == pVal || NULL == pVal->value)
                    return E_INVALIDARG;
                JIF(Convert((IProfile*)pVal->value,objTemp));
            }
        }
        else
        {
            if(it->second.isString())
            {
                string val = it->second;
                pProfile->Write(it->first.c_str(),val.c_str(),val.size()+1);
            }
            else if(true == it->second.isBoolean())
            {
                bool val = it->second;
                pProfile->Write(it->first.c_str(),val);
            }
            else if(it->second.isInteger())
            {
                int val = it->second;
                pProfile->Write(it->first.c_str(),val);
            }
            else if(true == it->second.isNumeric())
            {
                double val = it->second;
                pProfile->Write(it->first.c_str(),val);
            }
            else
            {
                LOG(0,"template stream: unknown type",it->second.toString().c_str());
            }
        }
    }
    return hr;
}

HRESULT FrameBufferInput(FrameSet& buf,IMediaFrame* pFrame,int64_t len,uint32_t& count)
{
    JCHK(NULL != pFrame,E_INVALIDARG);

    if(0 < len)
    {
        if(true == buf.empty())
            count = 0;

        buf.push_back(pFrame);
        if(0 != (pFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT))
        {
            ++count;
            if((len < 10000 && count > len) || (pFrame->info.dts - buf.front()->info.dts > len))
            {
                if(len < 10000)
                {
                    FrameIt it;
                    while((it = buf.begin()) != buf.end())
                    {
                        dom_ptr<IMediaFrame>& spFrame = *it;
                        if(0 != (spFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT))
                        {
                            if(count > len)
                            {
                                buf.erase(it);
                                --count;
                            }
                            else
                            {
                                spFrame->info.flag |= MEDIA_FRAME_FLAG_CORRUPT;
                                break;
                            }
                        }
                        else
                            buf.erase(it);
                    }
                }
                else
                {
                    FrameIt prev = buf.end();
                    uint32_t key_delta = 0;
                    for(FrameIt it = buf.begin() ; it != buf.end() ; ++it)
                    {
                        dom_ptr<IMediaFrame>& spFrame = *it;
                        if(0 != (spFrame->info.flag & MEDIA_FRAME_FLAG_SYNCPOINT))
                        {
                            if(pFrame->info.dts - spFrame->info.dts > len)
                            {
                                prev = it;
                                ++key_delta;
                            }
                            else
                            {
                                spFrame->info.flag |= MEDIA_FRAME_FLAG_CORRUPT;
                                break;
                            }
                        }
                    }
                    if(0 < key_delta)
                    {
                        count -= key_delta;
                        buf.erase(buf.begin(),prev);
                    }
                }
            }
        }
    }
    return S_OK;
}

//
//return 100ns
