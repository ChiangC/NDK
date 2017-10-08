#include "XAudioRecord.h"
#include <QAudioInput>
#include <iostream>
#include <list>


using namespace std;

class CXAudioRecord :public XAudioRecord
{
public:
	/*bool isExit = false;
	QMutex mutex;//互斥变量
	list<XData> datas;
	int maxList = 100;
	XData Pop()
	{
		mutex.lock();
		if (datas.empty())
		{
			mutex.unlock();
			return XData();
		}
		XData d = datas.front();
		datas.pop_front();
		mutex.unlock();
		return d;
	}*/

	void run()
	{
		cout << "进入音频录制线程" << endl;
		//一次读取一帧音频的字节数
		int readSize = nbSamples*channels*sampleByte;
		char *buf = new char[readSize];
		while (!isExit)
		{
			//读取已录制的音频
			//一次读取一帧音频
			if (audio_input->bytesReady() < readSize)
			{
				QThread::msleep(1);
				continue;
			}
			
			int size = 0;
			while (size != readSize)
			{
				int len = io->read(buf + size, readSize - size);
				if (len < 0)
				{
					break;
				}
				size += len;
			}
			if (size != readSize)
			{
				continue;
			}
			long long pts = GetCurTime();//微秒
			//已经读取一帧
			Push(XData(buf, readSize, pts));
			/*XData d;
			d.data = buf;
			d.size = readSize;
			mutex.lock();
			if (datas.size() > maxList){
				datas.front().Drop();
				datas.pop_front();
			}
			datas.push_back(d);
			mutex.unlock();*/
		}
		delete buf;
	}

	bool Init()
	{
		Stop();
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
		audio_input = new QAudioInput(audio_fmt);
		//开始录制音频
		io = audio_input->start();
		if (!io) return false;
		/*QThread::start();
		isExit = false;*/
		return true;
	}

	void Stop()
	{
		XDataThread::Stop();
		//isExit = true;
		//wait();//等待当前线程退出
		if (audio_input)
		{
			audio_input->stop();
		}

		if (io)
		{
			io->close();
		}
		audio_input = NULL;
		io = NULL;
	}

	QAudioInput *audio_input = NULL;
	QIODevice *io = NULL;
};

XAudioRecord *XAudioRecord::Get(XAUDIOTYPE type, unsigned char index)
{
	static CXAudioRecord record[255];
	return &record[index];
}

XAudioRecord::XAudioRecord()
{
}


XAudioRecord::~XAudioRecord()
{
}
