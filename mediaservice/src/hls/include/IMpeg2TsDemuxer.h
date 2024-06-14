#ifndef I_MPEG2_TS_DEMUXER
#define I_MPEG2_TS_DEMUXER

#ifdef WIN32
#ifndef LIBMPEGTS_DLL_EXPORTS
#define DllExport   __declspec( dllimport )
#else
#define DllExport   __declspec( dllexport )
#endif
#else
#define DllExport
#endif
class DllExport IMpeg2TsDemuxer
{
public:
	IMpeg2TsDemuxer(void);
	virtual ~IMpeg2TsDemuxer(void);
};
#endif
