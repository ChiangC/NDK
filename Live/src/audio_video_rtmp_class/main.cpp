
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
	int sampleByte = 2;//�ֽ���
	int nbSamples = 1024;

	//�������
	XVideoCapture *xvc = XVideoCapture::Get(0);
	if (!xvc->Init(0))
	{
		cout << "Open camera failed." << endl;
		return -1;
	}
	cout << "Open camera success." << endl;

	//1.qt��Ƶ��ʼ¼��
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

	//����Ƶ������
	XMediaEncode *xMediaEncode = XMediaEncode::Get(0);
	//2.��ʼ����ʽת��������
	//��ʼ����Ƶ��������ݽṹ
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

	//2.��Ƶ�ز����������ĳ�ʼ��
	xMediaEncode->channels = channels;
	xMediaEncode->nbSamples = 1024;
	xMediaEncode->sampleRate = sampleRate;
	xMediaEncode->inSampleFmt = XSampleFMT::X_S16;
	xMediaEncode->outSampleFmt = XSampleFMT::X_FLTP;
	if (!xMediaEncode->InitResample())
	{
		return -1;
	}

	///4.��ʼ����Ƶ������
	if (!xMediaEncode->InitVideoCodec())
	{
		return -1;
	}

	///4.��ʼ����Ƶ������
	if (!xMediaEncode->InitAudioCodec())
	{
		return -1;
	}

	//5.�����װ������Ƶ������
	//a.���������װ��������
	XRtmp *xRtmp = XRtmp::Get(0);
	if (!xRtmp->Init(outUrl))
	{
		return -1;
	}

	//b.�����Ƶ��
	if (!xRtmp->AddStream(xMediaEncode->video_codec_ctx))
	{
		return -1;
	}

	//b.�����Ƶ��
	if (!xRtmp->AddStream(xMediaEncode->audio_codec_ctx))
	{
		return -1;
	}

	////д���װͷ
	if (!xRtmp->SendHeader())
	{
		return -1;
	}

	/*char buf[4096] = {0};*/
	for (;;)
	{
		//һ�ζ�ȡһ֡��Ƶ
		XData d = audio_record->Pop();
		if (d.size <= 0)
		{
			QThread::msleep(1);
			continue;
		}

		//�Ѿ���һ֡Դ����
		//�ز���Դ����
		AVFrame *pcm = xMediaEncode->Resample(d.data);
		d.Drop();//���������ռ�

		AVPacket *pkt = xMediaEncode->EncodeAudio(pcm);

		if (!pkt) continue;

		////����
		xRtmp->SendFrame(pkt);

		
	}



	return a.exec();
}
