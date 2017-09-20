#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <iostream>

//预处理失败，找不到头文件；编译失败语法错误；链接失败，找不到lib文件；执行失败，找不到动态链接库。
using namespace cv;

#pragma comment(lib, "opencv_world320.lib")


int _main(int argc, char *argv)
{
    /*Mat image = imread("1.png");
    namedWindow("img");
    imshow("img", image);
    waitkey(0);*/

    Mat mat(3000, 4000, CV_8UC3);
    //mat.create(3000, 4000, CV_8UC3);

    int es = mat.elemSize();
    int size = mat.rows*mat.cols*es;

    for(int i = 0; i < size; i += es){
        mat.data[i] = 255; //B
        mat.data[i + 1] = 0;//G
        mat.data[i + 2] = 0;//R
    }

    namedWindow("mat");
    imshow("mat", mat);
    waitKey(0);
    return 0;
}


int main(int argc, char *argv)
{
    VideoCapture cam;
    string url = "rtsp://";

    namedWindow("video");

    //if(cam.open(url)){//open network camera
    if(cam.open(0)){//open usb camera
        cout<< "open cam success!"<< endl;
    }else{
        cout << "open cam failed!" << endl;
        waitKey(0);
        return -1;
    }

    Mat frame;
    while(true){
        cam.read(frame);
        imshow("video", frame);
        waitkey(1);
    }

    getchar();
    return 0;
}