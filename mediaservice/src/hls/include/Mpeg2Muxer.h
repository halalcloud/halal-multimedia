#ifndef WZD_MPEG2MUXER_H
#define WZD_MPEG2MUXER_H


#include "base.h"
#include "IMpeg2Muxer.h"

namespace wzd { struct PESPacket; } 

namespace wzd {

class CMpeg2Muxer : public IMpeg2Muxer 
{
public:
    explicit CMpeg2Muxer() throw();

    virtual ~CMpeg2Muxer() throw();

    virtual void Init() throw();

    /**
     * Ð´PES°ü
     * @param[in]
     * @param[in]
     * @return 
     * @exception
     */
    virtual void WritePESPacket(const PESPacket & pesInfo, const void* & buf) throw();

    /**
     * Ð´ES°ü
     * @param[in]
     * @param[in]
     * @return 
     * @exception
     */
    virtual void WriteESFrame(const sEsInfo & esInfo, const unsigned char* & buf) throw();


private:
    void __SystemHeader() throw();

    void __PackHeader() throw();

    void __ProgramStreamMap() throw();

    void __ProgramStreamDirectory() throw();

};

} // namespace wzd
#endif
