#include "common.h"

//1.ע�����еķ�װ��

//2.����װ��ʽ��������Ƶ�ļ�

//3.��ȡ������Ƶ�ļ��е�����Ϣ

//4.�����ļ�����ȡ����ļ��ľ��

//5.������������ļ��е�����Ϣ�����Ұ�������ӵ�����ļ���ȥ

//6.������ļ�

//*************��ʼ������ļ�д������*************
//7.����д������ļ���ͷ���� Allocate the stream private data and write the stream header to
// an output media file.

//8.ͨ��ѭ�����������ļ��ж�ȡ����Ƶ��������д�뵽����ļ�

//9.������ļ�д��β����

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

	AVOutputFormat *ofmt = NULL;//�����ʽ
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	int ret = 0;
	AVBitStreamFilterContext * vbsf = NULL;

	//1.ע�����еķ�װ��
	av_register_all();
	

	//2.����װ��ʽ��������Ƶ�ļ�
	ret = avformat_open_input(&ifmt_ctx, ioFiles.inputName, NULL, NULL);
	if (ret < 0)
	{
		printf("Error:open input file failed.\n");
		goto end;
	}

	//3.��ȡ������Ƶ�ļ��е�����Ϣ
	ret = avformat_find_stream_info(ifmt_ctx, NULL);
	if (ret < 0)
	{
		printf("Error: Failed to retrieve input stream information.\n");
		goto end;
	}
	//��������ļ��Ĳ���
	av_dump_format(ifmt_ctx, 0 , ioFiles.inputName, 0);

	//4.�����ļ�����ȡ����ļ��ľ��
	//��������ļ������ڣ����Բ�����avformat_open_input,
	//�����ļ�����ȡ����ļ��ľ��
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, ioFiles.outputName);
	if (!ofmt_ctx)
	{
		printf("Error: Could not create output context.\n");
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	//5.������������ļ��е�����Ϣ�����Ұ�������ӵ�����ļ���ȥ
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
		//��outStream ����AVCodecContext
		ret = avcodec_copy_context(outStream->codec, inStream->codec);
		outStream->codec->codec_tag = 0;//?
		//�����global header������globalheader��ֵ
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			outStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}
	av_dump_format(ofmt_ctx, 0, ioFiles.outputName, 1);

	vbsf = av_bitstream_filter_init("h264_mp4toannexb");

	//6.������ļ�
	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&ofmt_ctx->pb, ioFiles.outputName, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("Error: Could not open output file.\n");
			goto end;
		}
	}

	//*************��ʼ������ļ�д������*************
	//7.����д������ļ���ͷ���� Allocate the stream private data and write the stream header to
	// an output media file.
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0)
	{
		printf("Error:Could not write output file header.\n");
		goto end;
	}

	//8.ͨ��ѭ�����������ļ��ж�ȡ����Ƶ��������д�뵽����ļ�
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

		//��ͬʱ���֮���ת��,��ʱ�����һ��ʱ������������һ��ʱ��
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

		//������ļ�д��packet��ֵ
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if (ret < 0)
		{
			fprintf(stderr, "Error muxing packet.\n");
		}
		av_packet_unref(&pkt);
	}

	//9.������ļ�д��β����
	av_write_trailer(ofmt_ctx);

end:
	avformat_close_input(&ifmt_ctx);

	//������ļ����
	if (ofmt_ctx && !(ofmt_ctx->flags & AVFMT_NOFILE))
	{
		//�ر�����ļ�
		avio_closep(&ofmt_ctx->pb);
	}

	//�ͷŷ���õ�����ļ����
	avformat_free_context(ofmt_ctx);

	av_bitstream_filter_close(vbsf);
	vbsf = NULL;

	return 0;
}