#include "VideoDecoding.h"



FILE *pFin = NULL;
FILE *pFout = NULL;

AVCodec *pCodec = NULL;
AVCodecContext *pCodecContext = NULL;
AVCodecParserContext *pCodecParserCtx = NULL;//解析码流生成可以供解码器解码的包
AVFrame *frame = NULL;
AVPacket pkt;

static int open_input_output_file(char **argv)
{
	const char* inputFileName = argv[1];
	const char* outputFileName = argv[2];
	fopen_s(&pFin, inputFileName, "rb");
	if (!pFin)
	{
		printf("Open input file failed.\n");
		return -1;
	}

	fopen_s(&pFout, outputFileName, "wb");
	if (!pFout)
	{
		printf("Open output file failed.\n");
		return -1;
	}

	return 0;
}

static int open_decoder()
{
	avcodec_register_all();//注册所有与编解码相关的组件

	av_init_packet(&pkt);

	pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);

	if (!pCodec)
	{
		printf("Error:find codec failed.\n");
		return -1;
	}

	pCodecContext = avcodec_alloc_context3(pCodec);
	if (!pCodecContext)
	{
		printf("Error:alloc codecCtx failed.\n");
		return -1;
	}

	if (pCodec->capabilities & AV_CODEC_CAP_TRUNCATED)
	{
		pCodecContext->flags |= AV_CODEC_FLAG_TRUNCATED;
	}

	pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264);
	if (!pCodecParserCtx)
	{
		printf("Error: init parser failed.\n");
		return -1;
	}

	if (avcodec_open2(pCodecContext, pCodec, NULL) < 0)
	{
		printf("Error:Opening codec failed.\n");
		return -1;
	}

	frame = av_frame_alloc();
	if (!frame)
	{
		printf("Error:Could not allocate video frame.\n");
		return -1;
	}
	return 0;
}

static void write_out_yuv_frame(AVFrame *frame)
{
	uint8_t **pBuf = frame->data;
	int *pStride = frame->linesize;

	int color_idx;
	for (color_idx = 0; color_idx < 3; color_idx++)
	{
		int nWidth = color_idx == 0 ? frame->width : frame->width / 2;
		int nHeight = color_idx == 0 ? frame->height : frame->height / 2;
		
		int idx;
		for (idx = 0; idx < nHeight; idx++)
		{
			fwrite(pBuf[color_idx], 1, nWidth, pFout);
			pBuf[color_idx] += pStride[color_idx];
		}
		fflush(pFout);//刷新一下输出的文件

	}
}

static void close()
{
	fclose(pFin);
	fclose(pFout);

	avcodec_close(pCodecContext);
	av_free(pCodecContext);
	av_frame_free(&frame);
}

int main(int argc, char **argv)
{
	uint8_t inbuf[IN_BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	
	memset(inbuf + IN_BUFFER_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

	if (open_input_output_file(argv) < 0)
	{
		return -1;
	}

	if (open_decoder() < 0)
	{
		return -1;
	}

	int uDataSize = 0, len;//一次读到的缓存数据的长度
	uint8_t *pDataPtr = NULL;
	int got_frame = 0;

	while (1)
	{
		uDataSize = fread_s(inbuf, IN_BUFFER_SIZE, 1, IN_BUFFER_SIZE, pFin);
		if (uDataSize == 0)
		{
			break;
		}

		pDataPtr = inbuf;

		while (uDataSize > 0)
		{
			len = av_parser_parse2(pCodecParserCtx, pCodecContext, &pkt.data, &pkt.size,
			pDataPtr, uDataSize, 
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

			pDataPtr += len;
			uDataSize -= len;

			//判断数据包是否解析完成
			if (pkt.size == 0)
			{
				continue;//继续解析缓存中剩余的部分
			}

			//成功解析出一个packet的码流
			printf("Parser 1 packet.\n");

			int ret = avcodec_decode_video2(pCodecContext, frame, &got_frame, &pkt);
			if (ret < 0)
			{
				printf("Error:decode failed.\n");
				return -1;
			}

			if (got_frame)
			{
				printf("Decoded 1 frame OK! Width x Height:(%d x %d)\n", frame->width, frame->height);
				write_out_yuv_frame(frame);
			}
			
		}
	}

	pkt.data = NULL;
	pkt.size = 0;

	while (1)
	{
		int ret = avcodec_decode_video2(pCodecContext, frame, &got_frame, &pkt);
		if (ret < 0)
		{
			printf("Error:decode failed.\n");
			return -1;
		}

		if (got_frame)
		{
			printf("Flush decoder:Decoded 1 frame OK! Width x Height:(%d x %d)\n", frame->width, frame->height);
			write_out_yuv_frame(frame);
		}
		else{
			break;
		}
	}

	close();

	return 0;
}