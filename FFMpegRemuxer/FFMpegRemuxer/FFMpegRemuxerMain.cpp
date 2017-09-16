#include "common.h"

static bool hello(int argc, char **argv, IOFiles &iofiles)
{
	printf("Command format: %s inputfile outputfile\n", argv[0]);
	if (argc != 3)
	{
		printf("Error: command line error, please re-check.\n");
		return false;
	}

	iofiles.inputName = argv[1];
	iofiles.outputName = argv[2];

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

	av_register_all();

	//����װ��ʽ��������Ƶ�ļ�
	if ((ret = avformat_open_input(&ifmt_ctx, ioFiles.inputName, NULL, NULL)) < 0)
	{
		printf("Error: open input file failed.\n");
		goto end;
	}

	//��ȡ������Ƶ�ļ��е�����Ϣ
	if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
	{
		printf("Error: Failed to retrieve input stream information.\n");
		goto end;
	}

	//��������ļ��Ĳ���
	av_dump_format(ifmt_ctx, 0, ioFiles.inputName, 0);

	//��������ļ������ڣ����Բ�����avformat_open_input��
	//��Ҫ��avformat_alloc_output_context2
	//�����ļ�����ȡ����ļ��ľ��
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, ioFiles.outputName);
	if (!ofmt_ctx)
	{
		printf("Error: Could not create output context.\n");
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	//������������ļ��е�����Ϣ�����Ұ�������ӵ�����ļ���ȥ
	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		AVStream *inStream = ifmt_ctx->streams[i];
		//
		AVStream *outStream = avformat_new_stream(ofmt_ctx, inStream->codec->codec);
		if (!outStream)
		{
			printf("Error: Could not allocate output stream.\n");
			goto end;
		}

		//��outStream����AVCodecContext
		ret = avcodec_copy_context(outStream->codec, inStream->codec);
		outStream->codec->codec_tag = 0;
		//�����global header������globalheader��ֵ
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		{
			outStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
	}

	vbsf = av_bitstream_filter_init("h264_mp4toannexb");

	av_dump_format(ofmt_ctx, 0, ioFiles.outputName, 1);

	//ֻҪָ�����ǲ����ɣ��ʹ�����ļ�
	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&ofmt_ctx->pb, ioFiles.outputName, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("Error:Could not open output file.\n");
			goto end;
		}
	}

	//��ʼ������ļ�д������
	//����д������ļ���ͷ�ļ�
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret <0)
	{
		printf("Error:Could not write output file header.\n");
		goto end;
	}

	//ͨ��ѭ�����������ļ��ж�ȡ����Ƶ��������д�뵽����ļ�
	while (true)
	{
		AVStream *in_stream, *out_stream;

		//
		ret = av_read_frame(ifmt_ctx, &pkt);

		if (ret < 0)
		{
			break;
		}

		in_stream = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];

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

		//������ļ���д��packetֵ��
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if (ret < 0)
		{
			fprintf(stderr, "Error muxing pakcet.\n");
			break;
		}
		av_packet_unref(&pkt);
	}

	//������ļ�д��β����
	av_write_trailer(ofmt_ctx);

end:
	avformat_close_input(&ifmt_ctx);

	/*close output*/
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

	if (ret < 0 && ret != AVERROR_EOF)
	{
		fprintf(stderr, "Error failed to write packet to output file.\n");
		return 1;
	}

	return 0;
}