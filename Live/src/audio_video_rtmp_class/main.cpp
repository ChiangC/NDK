
#include <QtCore/QCoreApplication>
#include <iostream>
#include <qthread.h>
#include "XMediaEncode.h"
#include "XRtmp.h"
#include "XAudioRecord.h"
#include "XVideoCapture.h"


//extern "C"
//{
//#include <libswresample/swresample.h>
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//}
//#pragma comment(lib, "swresample.lib")
//#pragma comment(lib, "avutil.lib")
//#pragma comment(lib, "avcodec.lib")
//#pragma comment(lib, "avformat.lib")

using namespace std;

int main(int argc, char *argv[])
{
	const char *outUrl = "rtmp://106.14.33.215:1935/live/fmlive";

	QCoreApplication a(argc, argv);
	
	int ret = 0;

	int sampleRate = 44100;
	int channels = 2;
	int sampleByte = 2;//字节数
	int nbSamples = 1024;

	//打开摄像机
	XVideoCapture *xvc = XVideoCapture::Get(0);
	if (!xvc->Init(0))
	{
		cout << "Open camera failed." << endl;
		return -1;
	}
	cout << "Open camera success." << endl;

	//1.qt音频开始录制
	XAudioRecord *audio_record = XAudioRecord::Get();
	audio_record->sampleRate = sampleRate;
	audio_record->channels = channels;
	audio_record->sampleByte = sampleByte;
	audio_record->nbSamples = nbSamples;
	if (!audio_record->Init())
	{
		cout << "XAudioRecord Init failed." << endl;
		return -1;
	}

	//音视频编码类
	XMediaEncode *xMediaEncode = XMediaEncode::Get(0);
	//2.初始化格式转换上下文
	//初始化视频输出的数据结构
	xMediaEncode->inWidth = xvc->width;
	xMediaEncode->inHeight = xvc->height;
	xMediaEncode->outWidth = xvc->width;
	xMediaEncode->outHeight = xvc->height;
	if (!xMediaEncode->InitScale())
	{
		cout << "Init video scale failed." << endl;
		return -1;
	}
	cout << "Init video scale success." << endl;

	//2.音频重采样，上下文初始化
	xMediaEncode->channels = channels;
	xMediaEncode->nbSamples = 1024;
	xMediaEncode->sampleRate = sampleRate;
	xMediaEncode->inSampleFmt = XSampleFMT::X_S16;
	xMediaEncode->outSampleFmt = XSampleFMT::X_FLTP;
	if (!xMediaEncode->InitResample())
	{
		return -1;
	}

	///4.初始化视频编码器
	if (!xMediaEncode->InitVideoCodec())
	{
		return -1;
	}

	///4.初始化音频编码器
	if (!xMediaEncode->InitAudioCodec())
	{
		return -1;
	}

	//5.输出封装器和音频流配置
	//a.创建输出封装器上下文
	XRtmp *xRtmp = XRtmp::Get(0);
	if (!xRtmp->Init(outUrl))
	{
		return -1;
	}

	//b.添加视频流
	if (!xRtmp->AddStream(xMediaEncode->video_codec_ctx))
	{
		return -1;
	}

	//b.添加音频流
	if (!xRtmp->AddStream(xMediaEncode->audio_codec_ctx))
	{
		return -1;
	}

	////写入封装头
	if (!xRtmp->SendHeader())
	{
		return -1;
	}

	/*char buf[4096] = {0};*/
	for (;;)
	{
		//一次读取一帧音频
		XData d = audio_record->Pop();
		if (d.size <= 0)
		{
			QThread::msleep(1);
			continue;
		}

		//已经读一帧源数据
		//重采样源数据
		AVFrame *pcm = xMediaEncode->Resample(d.data);
		d.Drop();//用完就清理空间

		AVPacket *pkt = xMediaEncode->EncodeAudio(pcm);

		if (!pkt) continue;

		////推流
		xRtmp->SendFrame(pkt);

		
	}



	return a.exec();
}
