#include "stdafx.h"
#include "IntelVideoDecoder.h"
#include "IntelVideoEncoder.h"

#include <mfxvideo++.h>
DOM_CLASS_EXPORT_BEG
//check hardware if support export class else igore
//DOM_CLASS_EXPORT(CIntelVideoDecoder,FILTER_TYPE_TRANSFORM,MAKE_VERSION(0,0,0,0),(void*)CIntelVideoDecoder::CheckMediaType)

mfxIMPL impl;
MFXVideoSession m_mfxSession;

mfxInitParam initPar;
mfxVersion version;     // real API version with which library is initialized

MSDK_ZERO_MEMORY(initPar);

// we set version to 1.0 and later we will query actual version of the library which will got leaded
initPar.Version.Major = 1;
initPar.Version.Minor = 0;

initPar.GPUCopy = 0;

// try searching on all display adapters
initPar.Implementation = MFX_IMPL_HARDWARE_ANY;

JCHK(MFX_ERR_NONE == m_mfxSession.InitEx(initPar), false);

m_mfxSession.QueryIMPL(&impl);

if(MFX_IMPL_HARDWARE == MFX_IMPL_BASETYPE(impl))

{
    DOM_CLASS_EXPORT(CIntelVideoEncoder,FILTER_TYPE_TRANSFORM,MAKE_VERSION(0,0,0,0),(void*)CIntelVideoEncoder::CheckMediaType)
    LOG(0, "support hardware accelarated!");
}
else
{
    LOG(0, "not support hardware accelarated!");
}
DOM_CLASS_EXPORT_END
