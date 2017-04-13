#ifndef _STREAM_H
#define _STREAM_H

#include "ffmpeg_muxer.h"
#include "FileIO.h"

typedef struct _OutputStream
{
	AVStream *stream;

	AVFrame *videoFrame;
	AVFrame *audioFrame;

	int64_t next_pts;//��һ������Ƶ֡��ʾ��ʱ��

	float t, tincr, tincr2;
	struct SwsContext *sws_ctx;
	struct SwrContext *swr_ctx;
}OutputStream;

int add_audio_video_streams(OutputStream *video_stream, OutputStream audio_stream, AVFormatContext av_format_ctx, AVCodec *video_codec, AVCodec *audio_codec, FileIO &files);

#endif