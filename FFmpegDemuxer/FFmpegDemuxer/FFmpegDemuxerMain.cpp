#define __STDC_CONSTANT_MACROS

#include "FFmpegDemuxerMain.h"

const char * src_filename;
const char * video_dst_filename;
const char * audio_dst_filename;

FILE *pOutputAudio = NULL;
FILE *pOutputVideo = NULL;

AVFormatContext *av_format_ctx = NULL;
AVStream *av_stream = NULL;
AVStream *video_stream = NULL;
AVStream *audio_stream = NULL;
AVCodec *av_codec = NULL;
AVCodecContext *av_codec_ctx = NULL;
AVCodecContext *videoCodecCtx = NULL, *audioCodecCtx = NULL;

int frame_width = 0, frame_height = 0;
enum AVPixelFormat pix_fmt;

uint8_t *video_dst_data[4];
int video_dst_linesize[4];
int video_dst_buffer_size;

static int open_codec_context(enum AVMediaType type)
{
	int ret = 0;
	ret = av_find_best_stream(av_format_ctx, type, -1, -1, NULL, 0);

	if (ret < 0)
	{
		printf("Error:find stream failed.\n");
		return -1;
	}
	else
	{
		av_stream = av_format_ctx->streams[ret];
		av_codec_ctx = av_stream->codec;
		av_codec = avcodec_find_decoder(av_codec_ctx->codec_id);
	
		if (!av_codec)
		{
			printf("Error:cannot find decoder.\n");
			return -1;
		}
	
		if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0)
		{
			printf("Error:cannot open decoder.\n");
			return -1;
		}
	}
}

int main(int argc, char **argv)
{
	int ret;

	//=================FFmpeg相关初始化=================
	av_register_all();

	if (avformat_open_input(&av_format_ctx, src_filename, NULL, NULL) < 0)
	{
		printf("Error:open input file failed.\n");
		return -1;
	}

	if (avformat_find_stream_info(av_format_ctx, NULL) < 0)
	{
		printf("Error:find stream info failed.\n");
		return -1;
	}

	//=================打开音频和视频=================
	if (open_codec_context(AVMEDIA_TYPE_VIDEO) >= 0)
	{
		video_stream = av_stream;
		videoCodecCtx = av_codec_ctx;
		
		frame_width = videoCodecCtx->width;
		frame_height = videoCodecCtx->height;
		pix_fmt = videoCodecCtx->pix_fmt;

		ret = av_image_alloc(video_dst_data, video_dst_linesize, frame_width, frame_height, pix_fmt, 1);
		if (ret < 0)
		{
			printf("Error: Raw video buffer allocation failed.\n");
			return -1;
		}
		else{
			video_dst_buffer_size = ret;
		}

		pOutputVideo = fopen(video_dst_filename, "wb");
		if (!pOutputVideo)
		{
			printf("Error:Opening output yuv file failed.\n");
			return -1;
		}
	}



	return 0;

}