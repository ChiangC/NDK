#ifndef _VIDEO_DECODING_
#define _VIDEO_DECODING_

#define IN_BUFFER_SIZE 4096

extern "C"
{
#include "libavcodec\avcodec.h"
#include "libavutil\opt.h"
#include "libavutil\channel_layout.h"
#include "libavutil\common.h"
#include "libavutil\imgutils.h"
#include "libavutil\mathematics.h"
#include "libavutil\samplefmt.h"
}
#endif