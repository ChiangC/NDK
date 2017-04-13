#ifndef _FFMPEG_MUXER_H_
#define _FFMPEG_MUXER_H_

#define snprintf(buf, len, format, ...) _snprintf(buf, len, len, fromat, __VA_ARGS__)
extern "C"
{
#include "libavutil\timestamp.h"
#include "libavformat\avformat.h"
}
#endif


