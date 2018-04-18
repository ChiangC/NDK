// bilateral.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "iostream"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#pragma comment(lib, "opencv_world320d.lib")

using namespace std;
using namespace cv;

int main(int argc, char**argv)
{
	Mat src = imread("001.jpg");
	if (!src.data)
	{
		cout << "open file failed!" << endl;
		getchar();
		return -1;
	}
	namedWindow("src");
	moveWindow("src", 100, 100);
	imshow("src", src);

	Mat image;
	int d = 3;

	//named window不能创建多次
	namedWindow("image");
	moveWindow("image", 600, 100);

	for (;;)
	{
		
		long long b = getTickCount();//获取CPU跳数(从机器运行时)
		bilateralFilter(src, image, d, d * 2, d / 2);
		double sec = (double)(getTickCount() - b) / (double)getTickFrequency();
		cout <<"d="<< d<<" cost time:" << sec << endl;
		imshow("image", image);
		int key = waitKey(0);//0 means forever
		if (key == 'd') d += 2;
		else if (key == 'f') d -= 2;
		else break;
		if (d<0)d = 0;
	}
	

	waitKey(0);

    return 0;
}

