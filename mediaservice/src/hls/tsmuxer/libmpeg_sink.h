#ifndef LIBMPEG_SINK_H
#define LIBMPEG_SINK_H

class muxer_sink
{
public:
    virtual int write(unsigned char* p, unsigned int size) = 0;
};

#endif // LIBMPEG_SINK_H
