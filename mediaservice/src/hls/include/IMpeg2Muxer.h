#ifndef WZD_IMPEG2MUXER_H
#define WZD_IMPEG2MUXER_H


#include "base.h"
namespace wzd { struct PESPacket; } 

namespace wzd {

/**
 * mpeg2复用接口
 */
class IMpeg2Muxer 
{
public:
    inline explicit IMpeg2Muxer() throw();

    inline virtual ~IMpeg2Muxer() throw();

    virtual void Init() throw() = 0;

    /**
     * 写PES包
     * @param[in]
     * @param[in]
     * @return 
     * @exception
     */
    virtual void WritePESPacket(const PESPacket & pesInfo, const void* & buf) throw() = 0;

    /**
     * 写ES包
     * @param[in]
     * @param[in]
     * @return 
     * @exception
     */
    virtual void WriteESFrame(const sEsInfo & esInfo, const unsigned char* & buf) throw() = 0;

};
inline IMpeg2Muxer::IMpeg2Muxer() throw() 
{

}

inline IMpeg2Muxer::~IMpeg2Muxer() throw() 
{

}


} // namespace wzd
#endif
