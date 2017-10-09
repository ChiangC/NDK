#include "XVideoCapture.h"
#include <opencv2/highgui.hpp>
#include <iostream>

#include "XData.h"

#pragma comment(lib, "opencv_world320d.lib")

using namespace cv;
using namespace std;

class CXVideoCapture :public XVideoCapture
{
public:
	VideoCapture camera;

	void run()
	{
		Mat frame;
		while (!isExit)
		{
			if (!camera.read(frame))
			{
				msleep(1);
				continue;
			}

			if (frame.empty())
			{
				msleep(1);
				continue;
			}

			fmutex.lock();
			for(int i = 0; i < filters.size(); i++)
			{
			    Mat dest;
			    filters[i]->Filter(&frame, &dest);
			    frame = dest;
			}

			fmutex.unlock();

			//ȷ�������������ģ��������Ļ���Ҫ��ͨ��frameһ��һ�п���
			XData d((char*)frame.data, frame.cols*frame.rows*frame.elemSize(), GetCurTime());
			Push(d);
		}
	}

	bool Init(int camIndex = 0)
	{
		//open camera
		camera.open(camIndex);
		if (!camera.isOpened())
		{
			cout << "camera open failed." << endl;
			return false;
		}

		cout << camIndex << " camera open success" << endl;
		width = camera.get(CAP_PROP_FRAME_WIDTH);
		height = camera.get(CAP_PROP_FRAME_HEIGHT);
		fps = camera.get(CAP_PROP_FPS);
		if (fps == 0)fps = 25;

		return true;
	}

	bool Init(const char *url)
	{
		//open camera
		camera.open(url);
		if (!camera.isOpened())
		{
			cout << "camera open failed." << endl;
			return false;
		}

		cout << url << " camera open success" << endl;
		width = camera.get(CAP_PROP_FRAME_WIDTH);
		height = camera.get(CAP_PROP_FRAME_HEIGHT);
		fps = camera.get(CAP_PROP_FPS);
		if (fps == 0)fps = 25;

		return true;
	}

	void Stop()
	{
		XDataThread::Stop();

		if (camera.isOpened())
		{
			camera.release();
		}
	}
};

XVideoCapture *XVideoCapture::Get(unsigned char index)
{
	static CXVideoCapture xvc[255];
	return &xvc[index];
}

XVideoCapture::XVideoCapture()
{
}


XVideoCapture::~XVideoCapture()
{
}
