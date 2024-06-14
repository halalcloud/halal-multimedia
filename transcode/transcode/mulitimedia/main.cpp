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
#include "FilterGraph.h"
#include "GraphBuilder.h"
#include <time_expend.cpp>
DOM_CLASS_EXPORT_BEG
DOM_CLASS_EXPORT(CMediaFrame,"MediaFrame",MAKE_VERSION(0,0,0,0),NULL)
DOM_CLASS_EXPORT(CMediaFrameAllocate,"MediaFrameAllocate",MAKE_VERSION(0,0,0,0),NULL)
DOM_CLASS_EXPORT(CInputPin,"InputPin",MAKE_VERSION(0,0,0,0),NULL)
DOM_CLASS_EXPORT(COutputPin,"OutputPin",MAKE_VERSION(0,0,0,0),NULL)
DOM_CLASS_EXPORT(CFilterGraph,"FilterGraph",MAKE_VERSION(0,0,0,0),NULL)
DOM_CLASS_EXPORT(CGraphBuilder,"GraphBuilder",MAKE_VERSION(0,0,0,0),NULL)
DOM_CLASS_EXPORT_END

//return 100ns
