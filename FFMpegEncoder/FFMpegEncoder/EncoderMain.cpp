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

	//注册所需要的FFmpeg编码器组件
	avcodec_register_all();
	
	//查找AVCodec编解码器
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		return FF_ERROR_INITIALIZATION_FAILED;
	}

	//分配AVCodecContext实例
	codecCtx = avcodec_alloc_context3(codec);
	if (!codecCtx)
	{
		return FF_ERROR_INITIALIZATION_FAILED;
	}

	//设置编码器参数
	codecCtx->width = frameWidth;
	codecCtx->height = frameHeight;
	codecCtx->bit_rate = bitRate;
	AVRational r = {1,25};//每秒25帧
	codecCtx->time_base = r;
	codecCtx->gop_size = 12;//
	codecCtx->max_b_frames = 1;
	codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;//图像编码格式
	//自定义的私有数据
	av_opt_set(codecCtx->priv_data, "preset", "slow", 0);
	
	//打开编码器
	if (avcodec_open2(codecCtx, codec, NULL) < 0)
	{
		return FF_ERROR_INITIALIZATION_FAILED;
	}
	
	//分配AVFrame以及像素存储空间
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
		//初始化AVPacket
		av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;

		//读取数据到AVFrame
		read_yuv_data(0);
		read_yuv_data(1);
		read_yuv_data(2);

		//表示这一帧应当被显示的时间
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

	//编码器中可能存在没有输出的数据
	for (got_packet = 1; got_packet; frameIdx++)
	{
		//传NULL后avcodec 将会编码其内部缓存的数据
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

	//收尾
	//关闭输入和输出文件
	fclose(pFin);
	fclose(pFout);

	//释放编码器组件的实例和缓存
	avcodec_close(codecCtx);
	av_free(codecCtx);
	av_freep(&frame->data[0]);//释放frame中保存像素的地址空间
	av_frame_free(&frame);
	return 0;
}

static int read_yuv_data(int color)
{
	//color = 0；Y分量
	//color = 1；U分量
	//color = 2；V分量

	int color_height = color == 0 ? frameHeight : frameHeight / 2;
	int color_width = color == 0 ? frameWidth : frameWidth / 2;
	int color_size = color_height * color_width;
	int color_stride = frame->linesize[color];

	if (color_width == color_stride)//表示颜色分量在AVFrame中是连续存放的
	{
		//data[0]表示亮度的存储空间；data[1]表示色度的存储空间
		fread_s(frame->data[color], color_size, 1, color_size, pFin);
	}
	else //表示颜色分量在AVFrame中不是连续存放的,内存空间的边缘有填充的数据
	{
		//只能逐行读取
		int row_idx;
		for (row_idx = 0; row_idx < color_height; row_idx++)
		{
			fread_s(frame->data[color] + row_idx*color_stride, color_width, 1, color_width, pFin);
		}
	}

	return color_size;
}