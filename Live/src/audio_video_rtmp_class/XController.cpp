#include "XController.h"
#include "XVideoCapture.h"
#include "XAudioRecord.h"
#include "XRtmp.h"
#include <iostream>

using namespace std;

void XController::run()
{
    while(!isExit)
    {

    }

}

//设定美颜参数
bool XController::Set(std::string key, double val)
{
    long long beginTime = GetCurTime();
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

    //启动音视频采集线程
    XVideoCapture::Get()->Start();
    XAudioRecord::Get()->Start();

    //4.初始化格式转换上下文;初始化输出的数据结构
    XMediaEncode::Get()->inWidth = XVideoCapture::Get()->width;
    XMediaEncode::Get()->inHeight = XVideoCapture::Get()->height;
    XMediaEncode::Get()->outWidth = XVideoCapture::Get()->width;
    XMediaEncode::Get()->outHeight = XVideoCapture::Get()->height;
    if(!XMediaEncode::Get()->InitScale())
    {
        cout<< "视频像素格式转换打开失败！"<<endl;
        return false;
    }
    cout<< "视频像素格式转换打开成功！"<<endl;

    //5.初始化音频重采样上下文
    XMediaEncode::Get()->channels = XAudioRecord::Get()->channels;
    XMediaEncode::Get()->nbSamples = XAudioRecord::Get()->nbSamples;
    XMediaEncode::Get()->sampleRate = XAudioRecord::Get()->sampleRate;
    if(!XMediaEncode::Get()->InitResample())
    {
        cout<< "音频重采样上下文初始化失败！"<<endl;
        return false;
    }
    cout<< "音频重采样上下文初始化成功！"<<endl;

    //6.初始化音频编码器
    if(!XMediaEncode::Get()->InitAudioCodec())
    {
        cout<< "初始化音频编码器失败"<<endl;
        return false;
    }

    //7.初始化视频编码器
    if(!XMediaEncode::Get()->InitVideoCodec())
    {
        cout<< "初始化视频频编码器失败"<<endl;
        return false;
    }

    //8.创建输出封装器上下文
    if(!XRtmp::Get()->Init(outUrl.c_str()))
    {
        cout<<"创建输出封装器上下文失败！"<<endl;
        return false;
    }
    cout<<"创建输出封装器上下文成功！"<<endl;

    //9.添加视频流
    vindex = XRtmp::Get()->AddStream(XMediaEncode::Get()->video_codec_ctx);

    //10.添加音频流
    aindex = XRtmp::Get()->AddStream(XMediaEncode::Get()->audio_codec_ctx);

    if(vindex < 0 || aindex < 0)
    {
        cout<< "添加音视频流失败！"<<endl;
        return false;
    }
    cout<< "添加音视频流成功！"<<endl;

    //.send header
    if(!XRtmp::Get()->SendHeader())
    {
        cout<< "Rtmp send header failed."<<endl;
        return false;
    }
    cout<< "Rtmp send header success."<<endl;


    //清理已经已有的数据，以方便音视频同步;
    XVideoCapture::Get()->Clear();
    XAudioRecord::Get()->Clear();
    XDataThread::Start();
    return true;
}


void XController::Stop()
{
    XDataThread::Stop();
    //必须确保XController的线程退出
    XAudioRecord::Get()->Stop();
    XVideoCapture::Get()->Stop();
    XMediaEncode::Get()->Close();
    XRtmp::Get()->Close();

    cameraIndex = -1;
    inUrl = "";
    return;
}

XController::~XController()
{

}