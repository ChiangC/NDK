
#include <QtCore/QCoreApplication>
#include <iostream>
#include <qthread.h>
#include "XMediaEncode.h"
#include "XRtmp.h"
#include "XAudioRecord.h"
#include "XVideoCapture.h"
#include "XFilter.h"

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
	XFilter *xFilter = XFilter::Get();
    xFilter->Set("d", 9);

	if (!xvc->Init(0))
	{
		cout << "Open camera failed." << endl;
		return -1;
	}
	cout << "Open camera success." << endl;
	xvc->Start();

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
	audio_record->Start();

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
	int vindex = 0;
	vindex = xRtmp->AddStream(xMediaEncode->video_codec_ctx);
	if (vindex < 0)
	{
		return -1;
	}

	//b.�����Ƶ��
	int aindex = 0;
	aindex = xRtmp->AddStream(xMediaEncode->audio_codec_ctx);
	if (aindex < 0)
	{
		return -1;
	}

	////д���װͷ
	if (!xRtmp->SendHeader())
	{
		return -1;
	}

	//clear datas 
	xvc->Clear();
	audio_record->Clear();
	//start time
	long long beginTime = GetCurTime();

	/*char buf[4096] = {0};*/
	for (;;)
	{
		//һ�ζ�ȡһ֡��Ƶ
		XData ad = audio_record->Pop();
		XData vd = xvc->Pop();
		if (ad.size <= 0 && vd.size <= 0)
		{
			QThread::msleep(1);
			continue;
		}

		//������Ƶ
		//�Ѿ���һ֡Դ����
		if (ad.size > 0) {
			//�ز���Դ����
			ad.pts = ad.pts - beginTime;
			XData xData = xMediaEncode->Resample(ad);
			ad.Drop();//���������ռ�

			XData pkt = xMediaEncode->EncodeAudio(xData);
			if (pkt.size > 0) {
				////����
				if (xRtmp->SendFrame(pkt, aindex))
				{
					cout << "#" << flush;
				}
			}
		}
		
		//������Ƶ
		if (vd.size > 0)
		{
			vd.pts = vd.pts - beginTime;
			XData yuv = xMediaEncode->RGBToYUV(vd);
			vd.Drop();

			XData pkt = xMediaEncode->EncodeVideo(yuv);
			if (pkt.size > 0) {
				////����
				if (xRtmp->SendFrame(pkt, vindex))
				{
					cout << "%" << flush;
				}
			}
		}
		
	}

    XController::Get()->cameraIndex = 0;
    XController::Get()->outUrl = outUrl;
    XController::Get()->Start();
    long long beginTime = GetCurTime();
    XController::Get()->wait();

	return a.exec();
}
