#pragma once
#include "IMpeg2Muxer.h"
namespace wzd{
class CMpeg2Muxer : public IMpeg2Muxer
{
public:
	CMpeg2Muxer(void);
	virtual ~CMpeg2Muxer(void);
    /**
     * Ð´PES°ü
     */
    virtual void WritePESPacket(const PESPacket & pesInfo, const void* & buf) throw();

	/**
	 *	Ð´ESÖ¡
	 */
    virtual void WriteESFrame(const sEsInfo & esInfo, const unsigned char* & buf) throw();

private:
	void __SystemHeader();
	void __PackHeader();
	void __ProgramStreamMap();
	void __ProgramStreamDirectory();
};
};