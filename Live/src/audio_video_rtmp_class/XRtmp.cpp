#include "XRtmp.h"

#include <string>
#include <iostream>

extern "C"
{
#include <libavformat/avformat.h>
}

#pragma comment(lib, "avformat.lib")

using namespace std;

class CXRtmp :public XRtmp
{
public:
	void Close()
	{
		if (ofmt_ctx)
		{
			avformat_close_input(&ofmt_ctx);
			vstream = NULL;
		}
		video_codec_ctx = NULL;
		url = "";
	}

	bool Init(const char* url)
	{
		int ret = avformat_alloc_output_context2(&ofmt_ctx, 0, "flv", url);
		this->url = url;
		if (ret != 0){
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf)-1);
			cout<<buf<<endl;
			return false;
		}
		return true;
	}

	//Add video or audio stream
	//return stream index when success, -1 when failed.
	int AddStream(const AVCodecContext *codec_ctx)
	{
		if (!codec_ctx)
		{
			return -1;
		}
		

		//b.�����Ƶ��
		AVStream *stream = avformat_new_stream(ofmt_ctx, NULL);
		if (!stream){
			cout<<"avformat_new_stream failed"<<endl;
			return -1;
		}

		stream->codecpar->codec_tag = 0;

		//�ӱ��������Ʋ���
		avcodec_parameters_from_context(stream->codecpar, codec_ctx);
		av_dump_format(ofmt_ctx, 0, "", 1);

		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_codec_ctx = codec_ctx;
			vstream = stream;
		}
		else if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_codec_ctx = codec_ctx;
			astream = stream;
		}
		return stream->index;
	}

	bool SendHeader()
	{
		//��rtmp���������IO
		int ret = avio_open(&ofmt_ctx->pb, url.c_str(), AVIO_FLAG_WRITE);
		if (ret != 0){
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf)-1);
			cout<<buf<<endl;
			return false;
		}

		//д���װͷ
		//ע�⣺д���װͷ����ܻ��vstream��time_base�ĵ���
		//���Ե��õ�time_base��ʱ��һ��Ҫ������Ƶ���е�time_base�ٽ��м���
		ret = avformat_write_header(ofmt_ctx, NULL);
		if (ret != 0){
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf)-1);
			cout<<buf<<endl;
			return false;
		}
		return true;
	}

	bool SendFrame(XData d, int streamIndex)
	{
		if (!d.data || d.size <= 0)
		{
			return false;
		}
		AVPacket *pkt = (AVPacket*)d.data;
		pkt->stream_index = streamIndex;

		AVRational stime_base;
		AVRational dtime_base;
		//�ж�����Ƶ������Ƶ
		if (vstream && video_codec_ctx && pkt->stream_index == vstream->index)
		{
			stime_base = video_codec_ctx->time_base;
			dtime_base = vstream->time_base;
		}
		else if (astream && audio_codec_ctx && pkt->stream_index == astream->index)
		{
			stime_base = audio_codec_ctx->time_base;
			dtime_base = astream->time_base;
		}
		else{
			return false;
		}
		///push stream
		pkt->pts = av_rescale_q(pkt->pts, stime_base, dtime_base);
		pkt->dts = av_rescale_q(pkt->dts, stime_base, dtime_base);
		pkt->duration = av_rescale_q(pkt->duration, stime_base, dtime_base);
		int ret = av_interleaved_write_frame(ofmt_ctx, pkt);
		return true;
	}


private:
	string url = "";
	AVFormatContext *ofmt_ctx = NULL;
	const AVCodecContext *video_codec_ctx = NULL;
	const AVCodecContext *audio_codec_ctx = NULL;
	AVStream *vstream = NULL;
	AVStream *astream = NULL;
};

XRtmp* XRtmp::Get(unsigned char index)
{
	static CXRtmp cXRtmp[255];
	static bool isFirst = true;
	if (isFirst)
	{
		//ע�����еķ�װ��
		av_register_all();

		//ע����������Э��
		avformat_network_init();

		isFirst = false;
	}
	return &cXRtmp[0];
}

XRtmp::XRtmp()
{
}


XRtmp::~XRtmp()
{
}
