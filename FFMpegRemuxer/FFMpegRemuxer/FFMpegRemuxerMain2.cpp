#include "common.h"

//1.注册所有的封装器

//2.按封装格式打开输入视频文件

//3.获取输入视频文件中的流信息

//4.按照文件名获取输出文件的句柄

//5.逐个遍历输入文件中的流信息，并且把它们添加到输出文件中去

//6.打开输出文件

//*************开始往输出文件写入数据*************
//7.首先写入输出文件的头数据 Allocate the stream private data and write the stream header to
// an output media file.

//8.通过循环，从输入文件中读取音视频包，并且写入到输出文件

//9.向输出文件写入尾数据

static bool hello(int argc, char **argv, IOFiles &ioFiles)
{
	printf("Command format: %s inputfile outputfile\n", argv[0]);
	if (argc != 3)
	{
		printf("Error: command line error, please re-check.\n");
		return false;
	}

	ioFiles.inputName = argv[1];
	ioFiles.outputName = argv[2];

	return true;
}

int main(int argc, char **argv)
{
	IOFiles ioFiles;
	if (!hello(argc, argv, ioFiles))
	{
		return -1;
	}

	AVOutputFormat *ofmt = NULL;//输出格式
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	int ret = 0;
	AVBitStreamFilterContext * vbsf = NULL;

	//1.注册所有的封装器
	av_register_all();
	

	//2.按封装格式打开输入视频文件
	ret = avformat_open_input(&ifmt_ctx, ioFiles.inputName, NULL, NULL);
	if (ret < 0)
	{
		printf("Error:open input file failed.\n");
		goto end;
	}

	//3.获取输入视频文件中的流信息
	ret = avformat_find_stream_info(ifmt_ctx, NULL);
	if (ret < 0)
	{
		printf("Error: Failed to retrieve input stream information.\n");
		goto end;
	}
	//输出输入文件的参数
	av_dump_format(ifmt_ctx, 0 , ioFiles.inputName, 0);

	//4.按照文件名获取输出文件的句柄
	//由于输出文件不存在，所以不能用avformat_open_input,
	//按照文件名获取输出文件的句柄
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, ioFiles.outputName);
	if (!ofmt_ctx)
	{
		printf("Error: Could not create output context.\n");
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	//5.逐个遍历输入文件中的流信息，并且把它们添加到输出文件中去
	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		AVStream *inStream = ifmt_ctx->streams[i];
		//Add a new stream to a media file.
		AVStream *outStream = avformat_new_stream(ofmt_ctx, inStream->codec->codec);
		if (!outStream)
		{
			printf("Error: Could not allocate output stream.\n");
			goto end;
		}
		//给outStream 生成AVCodecContext
		ret = avcodec_copy_context(outStream->codec, inStream->codec);
		outStream->codec->codec_tag = 0;//?
		//如果有global header就设置globalheader的值
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			outStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}
	av_dump_format(ofmt_ctx, 0, ioFiles.outputName, 1);

	vbsf = av_bitstream_filter_init("h264_mp4toannexb");

	//6.打开输出文件
	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&ofmt_ctx->pb, ioFiles.outputName, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("Error: Could not open output file.\n");
			goto end;
		}
	}

	//*************开始往输出文件写入数据*************
	//7.首先写入输出文件的头数据 Allocate the stream private data and write the stream header to
	// an output media file.
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0)
	{
		printf("Error:Could not write output file header.\n");
		goto end;
	}

	//8.通过循环，从输入文件中读取音视频包，并且写入到输出文件
	while (true)
	{
		AVStream *in_stream, *out_stream;
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0)
		{
			break;
		}

		in_stream = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];

		//不同时间基之间的转换,把时间戳从一个时基调整到另外一个时基
		//copy packet
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
			(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
			(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;

		if (pkt.stream_index == 0) {

			AVPacket fpkt = pkt;
			int a = av_bitstream_filter_filter(vbsf,
				out_stream->codec, NULL, &fpkt.data, &fpkt.size,
				pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
			pkt.data = fpkt.data;
			pkt.size = fpkt.size;

		}

		//向输出文件写入packet的值
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if (ret < 0)
		{
			fprintf(stderr, "Error muxing packet.\n");
		}
		av_packet_unref(&pkt);
	}

	//9.向输出文件写入尾数据
	av_write_trailer(ofmt_ctx);

end:
	avformat_close_input(&ifmt_ctx);

	//如果有文件输出
	if (ofmt_ctx && !(ofmt_ctx->flags & AVFMT_NOFILE))
	{
		//关闭输出文件
		avio_closep(&ofmt_ctx->pb);
	}

	//释放分配好的输出文件句柄
	avformat_free_context(ofmt_ctx);

	av_bitstream_filter_close(vbsf);
	vbsf = NULL;

	return 0;
}