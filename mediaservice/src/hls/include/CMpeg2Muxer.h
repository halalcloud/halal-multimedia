#pragma once
#include "IMpeg2Muxer.h"
namespace wzd{
class CMpeg2Muxer : public IMpeg2Muxer
{
public:
	CMpeg2Muxer(void);
	virtual ~CMpeg2Muxer(void);
    /**
     * дPES��
     */
    virtual void WritePESPacket(const PESPacket & pesInfo, const void* & buf) throw();

	/**
	 *	дES֡
	 */
    virtual void WriteESFrame(const sEsInfo & esInfo, const unsigned char* & buf) throw();

private:
	void __SystemHeader();
	void __PackHeader();
	void __ProgramStreamMap();
	void __ProgramStreamDirectory();
};
};