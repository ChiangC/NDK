#include "stdio.h"
#include "stdint.h"
#include "Error.h"

extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
}

const char *inputFileName = NULL;
const char *outputFileName = NULL;

int frameWidth = 0, frameHeight = 0;
int bitRate = 0, frameToEncode = 0;

AVCodec *codec = NULL;
AVCodecContext *codecCtx = NULL;
AVFrame *frame = NULL;
AVPacket pkt;

FILE *pFin = NULL;
FILE *pFout = NULL;

static int read_yuv_data(int color);

static int parse_input_paramaters(int argc, char **argv)
{
	inputFileName = argv[1];
	outputFileName = argv[2];
	fopen_s(&pFin, inputFileName, "rb");
	if (!pFin)
	{
		return IO_FILE_ERROR_OPEN_FAILED;
	}

	fopen_s(&pFout, outputFileName, "wb");
	if (!pFout)
	{
		return IO_FILE_ERROR_OPEN_FAILED;
	}
	frameWidth = atoi(argv[3]);
	frameHeight = atoi(argv[4]);
	bitRate = atoi(argv[5]);
	frameToEncode = atoi(argv[6]);
	return 1;

}
int main(int argc, char **argv)
{
	if (parse_input_paramaters(argc, argv) < 0)
	{
		printf("Parse input paramaters failed.\n");
		return -1;
	}

	int got_packet = 0;

	//ע������Ҫ��FFmpeg���������
	avcodec_register_all();
	
	//����AVCodec�������
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		return FF_ERROR_INITIALIZATION_FAILED;
	}

	//����AVCodecContextʵ��
	codecCtx = avcodec_alloc_context3(codec);
	if (!codecCtx)
	{
		return FF_ERROR_INITIALIZATION_FAILED;
	}

	//���ñ���������
	codecCtx->width = frameWidth;
	codecCtx->height = frameHeight;
	codecCtx->bit_rate = bitRate;
	AVRational r = {1,25};//ÿ��25֡
	codecCtx->time_base = r;
	codecCtx->gop_size = 12;//
	codecCtx->max_b_frames = 1;
	codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;//ͼ������ʽ
	//�Զ����˽������
	av_opt_set(codecCtx->priv_data, "preset", "slow", 0);
	
	//�򿪱�����
	if (avcodec_open2(codecCtx, codec, NULL) < 0)
	{
		return FF_ERROR_INITIALIZATION_FAILED;
	}
	
	//����AVFrame�Լ����ش洢�ռ�
	frame = av_frame_alloc();
	if (!frame)
	{
		return FF_ERROR_INITIALIZATION_FAILED;
	}
	frame->width = codecCtx->width;
	frame->height = codecCtx->height;
	frame->format = codecCtx->pix_fmt;
	if (av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, (AVPixelFormat)frame->format, 32) < 0)
	{
		return FF_ERROR_INITIALIZATION_FAILED;
	}

	int frameIdx;
	for (frameIdx = 0; frameIdx < frameToEncode; frameIdx++)
	{
		//��ʼ��AVPacket
		av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;

		//��ȡ���ݵ�AVFrame
		read_yuv_data(0);
		read_yuv_data(1);
		read_yuv_data(2);

		//��ʾ��һ֡Ӧ������ʾ��ʱ��
		frame->pts = frameIdx;

		int ret = avcodec_encode_video2(codecCtx, &pkt, frame, &got_packet);
		if (ret < 0)
		{
			printf("Error:encoding failed.\n");
			return FF_ERROR_ENCODING_FAILED;
		}

		if (got_packet)
		{
			printf("Write packet of frame %d, size = %d", frameIdx,pkt.size);
			fwrite(pkt.data, 1, pkt.size, pFout);
			av_packet_unref(&pkt);
		}

	}

	//�������п��ܴ���û�����������
	for (got_packet = 1; got_packet; frameIdx++)
	{
		//��NULL��avcodec ����������ڲ����������
		if (avcodec_encode_video2(codecCtx, &pkt, NULL, &got_packet))
		{
			printf("Error:encoding failed.\n");
			return FF_ERROR_ENCODING_FAILED;
		}

		if (got_packet)
		{
			printf("Write cached packet of frame %d, size = %d", frameIdx, pkt.size);
			fwrite(pkt.data, 1, pkt.size, pFout);
			av_packet_unref(&pkt);
		}
		/*else
		{
			break;
		}*/
	}

	//��β
	//�ر����������ļ�
	fclose(pFin);
	fclose(pFout);

	//�ͷű����������ʵ���ͻ���
	avcodec_close(codecCtx);
	av_free(codecCtx);
	av_freep(&frame->data[0]);//�ͷ�frame�б������صĵ�ַ�ռ�
	av_frame_free(&frame);
	return 0;
}

static int read_yuv_data(int color)
{
	//color = 0��Y����
	//color = 1��U����
	//color = 2��V����

	int color_height = color == 0 ? frameHeight : frameHeight / 2;
	int color_width = color == 0 ? frameWidth : frameWidth / 2;
	int color_size = color_height * color_width;
	int color_stride = frame->linesize[color];

	if (color_width == color_stride)//��ʾ��ɫ������AVFrame����������ŵ�
	{
		//data[0]��ʾ���ȵĴ洢�ռ䣻data[1]��ʾɫ�ȵĴ洢�ռ�
		fread_s(frame->data[color], color_size, 1, color_size, pFin);
	}
	else //��ʾ��ɫ������AVFrame�в���������ŵ�,�ڴ�ռ�ı�Ե����������
	{
		//ֻ�����ж�ȡ
		int row_idx;
		for (row_idx = 0; row_idx < color_height; row_idx++)
		{
			fread_s(frame->data[color] + row_idx*color_stride, color_width, 1, color_width, pFin);
		}
	}

	return color_size;
}