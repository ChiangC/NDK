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
	bool AddStream(const AVCodecContext *codec_ctx)
	{
		if (!codec_ctx)
		{
			return false;
		}
		

		//b.添加视频流
		AVStream *stream = avformat_new_stream(ofmt_ctx, NULL);
		if (!stream){
			cout<<"avformat_new_stream failed"<<endl;
			return false;
		}

		stream->codecpar->codec_tag = 0;

		//从编码器复制参数
		avcodec_parameters_from_context(stream->codecpar, codec_ctx);
		av_dump_format(ofmt_ctx, 0, "", 1);

		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_codec_ctx = codec_ctx;
			vstream = stream;
		}
	}

	bool SendHeader()
	{
		//打开rtmp的网络输出IO
		int ret = avio_open(&ofmt_ctx->pb, url.c_str(), AVIO_FLAG_WRITE);
		if (ret != 0){
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf)-1);
			cout<<buf<<endl;
			return false;
		}

		//写入封装头
		//注意：写入封装头后可能会把vstream的time_base改掉，
		//所以等用到time_base的时候一定要，拿视频流中的time_base再进行计算
		ret = avformat_write_header(ofmt_ctx, NULL);
		if (ret != 0){
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf)-1);
			cout<<buf<<endl;
			return false;
		}
		return true;
	}

	bool SendFrame(AVPacket *pkt)
	{
		if (NULL == pkt)
		{
			return false;
		}
		///推流
		pkt->pts = av_rescale_q(pkt->pts, video_codec_ctx->time_base, vstream->time_base);
		pkt->dts = av_rescale_q(pkt->dts, video_codec_ctx->time_base, vstream->time_base);

		int ret = av_interleaved_write_frame(ofmt_ctx, pkt);

		/*if (ret == 0){
			cout << "#" << flush;
		}*/
		return true;
	}


private:
	string url = "";
	AVFormatContext *ofmt_ctx = NULL;
	const AVCodecContext *video_codec_ctx = NULL;
	const AVCodecContext *audio_codec_ctx = NULL;
	AVStream *vstream = NULL;
};

XRtmp* XRtmp::Get(unsigned char index)
{
	static CXRtmp cXRtmp[255];
	static bool isFirst = true;
	if (isFirst)
	{
		//注册所有的封装器
		av_register_all();

		//注册所有网络协议
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
