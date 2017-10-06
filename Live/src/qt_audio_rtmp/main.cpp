
#include <QtCore/QCoreApplication>
#include <QAudioInput>
#include <iostream>
#include <qthread.h>

extern "C"
{
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")

using namespace std;

int main(int argc, char *argv[])
{
	const char *outUrl = "rtmp://106.14.33.215:1935/live/fmlive";

	QCoreApplication a(argc, argv);

	avcodec_register_all();

	av_register_all();

	avformat_network_init();

	int sampleRate = 44100;
	int channels = 2;
	int sampleByte = 2;//字节数
	AVSampleFormat inSampleFormat = AV_SAMPLE_FMT_S16;
	AVSampleFormat outSampleFormat = AV_SAMPLE_FMT_FLTP;

	//1.qt音频开始录制
	QAudioFormat audio_fmt;
	audio_fmt.setSampleRate(sampleRate);
	audio_fmt.setChannelCount(channels);
	audio_fmt.setSampleSize(sampleByte * 2);
	audio_fmt.setCodec("audio/pcm");
	audio_fmt.setByteOrder(QAudioFormat::LittleEndian);
	audio_fmt.setSampleType(QAudioFormat::UnSignedInt);
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
	if (!info.isFormatSupported(audio_fmt))
	{
		cout << "Audio format not supported!" << endl;
		audio_fmt = info.nearestFormat(audio_fmt);
	}
	QAudioInput *audio_input = new QAudioInput(audio_fmt);
	//开始录制音频
	QIODevice *io = audio_input->start();

	//音频重采样，上下文初始化
	SwrContext *swr_ctx = NULL;
	swr_ctx = swr_alloc_set_opts(swr_ctx,
		av_get_default_channel_layout(channels), outSampleFormat, sampleRate,//输出格式
		av_get_default_channel_layout(channels), inSampleFormat, sampleRate,//输入格式
		0,0);

	if (!swr_ctx)
	{
		cout << "swr_alloc_set_opts failed!" << endl;
	}

	int ret = swr_init(swr_ctx);
	if (ret != 0)
	{
		char err[1024] = {0};
		av_strerror(ret, err, sizeof(err) -1);
		cout << err << endl;
		return -1;
	}
	cout << "Audio resample context init success!" << endl;

	AVFrame *pcm = av_frame_alloc();
	pcm->format = outSampleFormat;
	pcm->channels = channels;
	pcm->channel_layout = av_get_default_channel_layout(channels);
	pcm->nb_samples = 1024;//一帧音频，一通道的采样数量
	ret = av_frame_get_buffer(pcm, 0);//给pcm分配存储空间
	if (ret != 0)
	{
		char err[1024] = { 0 };
		av_strerror(ret, err, sizeof(err)-1);
		cout << err << endl;
		return -1;
	}

	///4.初始化音频编码器
	AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!codec)
	{
		cout << "avcodec_find_encoder AV_CODEC_ID_AAC failed!" << endl;
		return -1;
	}

	AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx)
	{
		cout << "avcodec_alloc_context3 failed!" << endl;
		return -1;
	}
	cout << "avcode_alloc_context3 success!" << endl;
	
	codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	codec_ctx->thread_count = 8;
	codec_ctx->bit_rate = 40000;
	codec_ctx->sample_rate = sampleRate;
	codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
	codec_ctx->channels = channels;
	codec_ctx->channel_layout = av_get_default_channel_layout(channels);


	ret = avcodec_open2(codec_ctx, NULL, NULL);
	if (ret != 0)
	{
		char err[1024] = { 0 };
		av_strerror(ret, err, sizeof(err)-1);
		cout << err << endl;
		return -1;
	}

	//输出封装器和音频流配置
	//a.创建输出封装器上下文
	AVFormatContext *ofmt_ctx = NULL;
	ret = avformat_alloc_output_context2(&ofmt_ctx, 0, "flv", outUrl);
	if (ret != 0)
	{
		char err[1024] = { 0 };
		av_strerror(ret, err, sizeof(err)-1);
		cout << err << endl;
		return -1;
	}
	//b.添加音频流
	AVStream *astream = avformat_new_stream(ofmt_ctx, NULL);
	if (!astream)
	{
		char err[1024] = { 0 };
		av_strerror(ret, err, sizeof(err)-1);
		cout << err << endl;
		return -1;
	}
	astream->codecpar->codec_tag = 0;
	//从编码器复制参数
	avcodec_parameters_from_context(astream->codecpar, codec_ctx);
	av_dump_format(ofmt_ctx, 0, outUrl, 1);
	
	//打开rtmp的网络输出IO
	ret = avio_open(&ofmt_ctx->pb, outUrl, AVIO_FLAG_WRITE);
	if (ret != 0)
	{
		char err[1024] = { 0 };
		av_strerror(ret, err, sizeof(err)-1);
		cout << err << endl;
		return -1;
	}
	//写入封装头
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret != 0)
	{
		char err[1024] = { 0 };
		av_strerror(ret, err, sizeof(err)-1);
		cout << err << endl;
		return -1;
	}
	//一次读取一帧音频的字节数
	int readSize = pcm->nb_samples*channels*sampleByte;
	char *buf = new char[readSize];
	int apts = 0;
	AVPacket pkt = {0};

	/*char buf[4096] = {0};*/
	for (;;)
	{
		//一次读取一帧音频
		if (audio_input->bytesReady() < readSize)
		{
			QThread::msleep(1);
			continue;
		}
		int size = 0;
		while (size != readSize)
		{
			int len = io->read(buf+size, readSize - size);
			if (len < 0)
			{
				break;
			}
			size += len;
		}
		if (size != readSize)continue;

		//已经读一帧源数据
		//重采样源数据
		const uint8_t *indata[AV_NUM_DATA_POINTERS] = {0};
		indata[0] = (uint8_t *)buf;
		int len = swr_convert(swr_ctx, pcm->data, pcm->nb_samples,//输出参数，输出存储地址和样本数量
			indata, pcm->nb_samples);
		
		//pts运算
		//nb_sample / smaple_rate = 一帧音频的秒数sec
		//pts = sec*timebase.den

		apts += av_rescale_q(pcm->nb_samples, {1, sampleRate}, codec_ctx->time_base);

		ret = avcodec_send_frame(codec_ctx, pcm);
		if (ret != 0)continue;

		av_packet_unref(&pkt);
		ret = avcodec_receive_packet(codec_ctx, &pkt);
		if (ret != 0)
		{
			continue;
		}
		cout << pkt.size << " " << flush;

		//推流
		pkt.pts = av_rescale_q(pkt.pts, codec_ctx->time_base, astream->time_base);
		pkt.dts = av_rescale_q(pkt.dts, codec_ctx->time_base, astream->time_base);
		pkt.duration = av_rescale_q(pkt.duration, codec_ctx->time_base, astream->time_base);
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		
	}
	delete buf;





	return a.exec();
}
