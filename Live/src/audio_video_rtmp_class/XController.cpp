#include "XController.h"
#include "XVideoCapture.h"
#include "XAudioRecord.h"
#include "XRtmp.h"
#include <iostream>

using namespace std;

//设定美颜参数
bool XController::Set(std::string key, double val)
{
    XFilter::Get()->Set(key, val);
    return true;
}

bool XController::Start()
{
    //1.设置磨皮过滤器
    XVideoCaptrue::Get()->AddFilter(XFilter::Get());

    //2.open camera
    if(cameraIndex >= 0)
    {
        if(!XVideoCapture::Get()->Init(cameraIndex))
        {
            std::cout<< "Open camera failed."<<std::endl;
            return false;
        }
    }else if(!inUrl.empty())
    {
        if(!XVideoCapture::Get()->Init(inUrl))
        {
             std::cout<< "Open camera failed."<<std::endl;
             return false;
        }

    }else
    {
        std::cout<< "Please set camera params"<<std::endl;
        return false;
    }
    std::cout<< "Open camera success."<<std::endl;

    //3.start audio record
    if(!XAudioRecord::Get()->Init())
    {
        cout<< "Open audio record device failed."<<endl;
        return false;
    }
    cout<< "Open audio record device success."<<endl;

    //4.初始化格式转换上下文;初始化输出的数据结构
    XMediaEncode::Get()->inWidth = XVideoCapture::Get()->width;


    //5.初始化音频重采样上下文


    //6.初始化音频编码器


    //7.初始化视频编码器


    //8.创建输出封装器上下文


    //9.添加视频流


    //10.添加音频流


    //

    //.send header
    if(!XRtmp::Get()->SendHeader())
    {
        cout<< "Rtmp send header failed."<<endl;
        return false;
    }
    cout<< "Rtmp send header success."<<endl;

    return true;
}


void XController::Stop()
{
    cameraIndex = -1;
    inUrl = "";
    return;
}

XController::~XController()
{

}