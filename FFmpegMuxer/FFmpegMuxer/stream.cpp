#include "stream.h"

#define HAVE_VIDEO 1
#define HAVE_AUDIO 2

#define STREAM_FRAME_RATE 25

static int add_video_stream(OutputStream *video_stream, AVFormatContext *format_ctx, AVCodec **codec, enum AVCodecID codec_id, int frame_width, int frame_height)
{
	*codec = avcodec_find_encoder(codec_id);
	if (!*codec)
	{
		printf("Error:cannot find video encoder for video stream.\n");
		return -1;
	}

	video_stream->stream = avformat_new_stream(format_ctx, *codec);
	if (!(video_stream->stream))
	{
		printf("Error:cannot addd new video stream to video file.\n");
		return -1;
	}

	video_stream->stream->id = format_ctx->nb_streams - 1;
	AVCodecContext *codec_ctx = video_stream->stream->codec;

	codec_ctx->codec_id = codec_id;
	codec_ctx->bit_rate = 400000;
	codec_ctx->width = frame_width;
	codec_ctx->height = frame_height;
	AVRational r = {1, STREAM_FRAME_RATE};
	codec_ctx->time_base = r;
	codec_ctx->gop_size = 12;
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
	{
		/* just for testing, we also add B frames */
		c->max_b_frames = 2;
	}
	if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
	{
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		* This does not happen with normal video, it just happens here as
		* the motion of the chroma plane does not match the luma plane. */
		c->mb_decision = 2;
	}

	if (format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
	{
		codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
}

int add_audio_video_streams(OutputStream *video_stream, OutputStream audio_stream, AVFormatContext *av_format_ctx, AVCodec *video_codec, AVCodec *audio_codec, FileIO &files)
{
	int ret = 0;
	AVOutputFormat *av_output_fmt = av_format_ctx->oformat;
	
	if (av_output_fmt->video_codec != AV_CODEC_ID_NONE)
	{
		//ÃÌº” ”∆µ
		add_video_stream();

	}

	if (av_output_fmt->audio_codec != AV_CODEC_ID_NONE)
	{
		//ÃÌº”“Ù∆µ

	}
}
