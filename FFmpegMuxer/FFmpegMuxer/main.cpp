#include "common.h"


static bool hello(int argc, char **argv, FileIO &files)
{
	if (argc < 5)
	{
		printf("Usage:%s outputfile inputfile frame_width frame_height.\n", argv[0]);
		return false;
	}

	files.output_filename = argv[1];
	files.input_filename = argv[2];
	files.frame_width = atoi(argv[3]);
	files.frame_height = atoi(argv[4]);
}

static int open_encoder_muxer(AVOutputFormat **output_format, AVFormatContext **format_ctx, const char *output_filename)
{
	av_register_all();

	avformat_alloc_output_context2(format_ctx, NULL, NULL, output_filename);

	if (!format_ctx)
	{
		printf("Could not deduce output format from file extension: using MPEG.\n");
		avformat_alloc_output_context2(format_ctx, NULL, "mpeg", output_filename);
	}

	if (!format_ctx)
	{
		printf("Error:cannot alloc any format context.\n");
		return -1;
	}

	*output_format = (*format_ctx)->oformat;

	return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;
	FileIO files;
	if (!hello(argc, argv, files))
	{
		printf("Error:parsing command line failed.\n");
		return -1;
	}

	AVOutputFormat *output_format = NULL;
	AVFormatContext *format_ctx = NULL;
	AVCodec *video_codec = NULL, *audio_codec = NULL;
	OutputStream video_stream = { 0 }, audtio_stream = { 0 };

	//FFmpeg相关初始化
	open_encoder_muxer(&output_format, &format_ctx, files.output_filename);

	ret = add_audio_video_streams();

	int haveAudio = ret & HAVE_AUDIO, haveVideo = ret & HAVE_VIDEO;

	if (haveVideo)
	{
		open_video();
	}

	if (haveAudio)
	{
		open_audio();
	}

	//输出文件相关信息
	av_dump_format(format_ctx, 0, files.output_filename, 1);

	if (!(output_format->flags& AVFMT_NOFILE))//判断输出文件是否存在
	{
		//打开封装格式的输出文件
		ret = avio_open(&format_ctx->pb, files.output_filename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("Error:Opening output file failed.\n");
			return -1;
		}
	}

	//写入文件头
	ret = avformat_write_header(format_ctx, NULL);
	if (ret < 0)
	{
		printf("Error:writing file header failed.\n");
		return -1;
	}

}
