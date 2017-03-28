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
AVFrame *frame;
AVPacket packet;

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

int decode_packet(int &got_frame)
{
	int ret = 0;
	got_frame = 0;

	if (packet.stream_index == video_stream->index)
	{
		ret = avcodec_decode_video2(videoCodecCtx, frame, &got_frame, &packet);
		if (ret < 0)
		{
			printf("Error:decode video frame failed.\n");
			return -1;
		}

		if (got_frame)
		{
			av_image_copy(video_dst_data, video_dst_linesize, (const uint8_t **)frame->data, frame->linesize, pix_fmt, frame_width, frame_height);
			
			fwrite(video_dst_data[0], 1, video_dst_buffer_size, pOutputVideo);
		}
	}
	else if (packet.stream_index == audio_stream->index)
	{
		ret = avcodec_decode_audio4(audioCodecCtx, frame, &got_frame, &packet);
		if (ret < 0)
		{
			printf("Error:decode audio frame failed.\n");
			return -1;
		}

		if (got_frame)
		{
			size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
			fwrite(frame->extended_data[0], 1, unpadded_linesize, pOutputAudio);
		}
	}

	return FFMIN(ret, packet.size);
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

		fopen_s(&pOutputVideo, video_dst_filename, "wb");
		if (!pOutputVideo)
		{
			printf("Error:Opening output yuv file failed.\n");
			return -1;
		}
	}

	if (open_codec_context(AVMEDIA_TYPE_AUDIO) >= 0)
	{
		audio_stream = av_stream;
		audioCodecCtx = av_codec_ctx;

		fopen_s(&pOutputAudio, audio_dst_filename, "wb");
		if (!pOutputAudio)
		{
			printf("Error:Opening output acc file failed.\n");
			return -1;
		}
	}
	
	av_dump_format(av_format_ctx, 0, src_filename, 0);
	
	//=================读取处理音频和视频数据=================
	frame = av_frame_alloc();
	if (frame == NULL)
	{
		printf("Error:Allocating AVFrame failed.\n");
		return -1;
	}

	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;
	int got_frame = 0;
	while (av_read_frame(av_format_ctx, &packet) >=0)
	{
		do{
			ret = decode_packet(got_frame);
			packet.data += ret;
			packet.size -= ret;
		} while (packet.size > 0);
	}

	packet.data = NULL;
	packet.size = 0;

	do{
		ret = decode_packet(got_frame);
		packet.data += ret;
		packet.size -= ret;
	} while (got_frame);

	//=================收尾工作=================
	avcodec_close(videoCodecCtx);
	avcodec_close(audioCodecCtx);
	avformat_close_input(&av_format_ctx);
	av_frame_free(&frame);
	av_free(video_dst_data[0]);

	fclose(pOutputVideo);
	fclose(pOutputAudio);

	return 0;

}