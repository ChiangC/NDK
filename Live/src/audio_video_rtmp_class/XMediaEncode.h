#pragma once
#include "XData.h"

struct AVFrame;
struct AVPacket;
class AVCodecContext;

enum XSampleFMT
{
	X_S16 = 1,
	X_FLTP = 8
};
//音视频编码接口类
class XMediaEncode
{

public :
	//输入参数
    int inWidth = 1280;
    int inHeight = 720;
    int inPixSize = 3;
	int channels = 2;
	int sampleRate = 44100;
	XSampleFMT inSampleFmt = X_S16;

	//输出参数
    int outWidth = 1280;
    int outHeight = 720;
    int bitRate = 400000;//压缩后每秒视频的bit位大小,通过压缩率控制视频的码率；
	int fps = 25;
	XSampleFMT outSampleFmt = X_FLTP;
	int nbSamples = 1024;

    static XMediaEncode * Get(unsigned char index = 0);

	//返回值不需要调用者清理
    virtual XData RGBToYUV(XData xData) = 0;

	//初始化像素格式转换上下文
    virtual bool InitScale() = 0;

	//初始化音频采样上下文
	virtual bool InitResample() = 0;

	//返回值不需要调用者清理
	virtual XData Resample(XData d) = 0;

	//视频编码器初始化
    virtual bool InitVideoCodec() = 0;

	//音频编码器初始化
	virtual bool InitAudioCodec() = 0;

	//视频编码,返回值无需用户清理
	virtual XData EncodeVideo(XData xData) = 0;

	//音频编码,返回值无需用户清理
	virtual XData EncodeAudio(XData xData) = 0;
    
	virtual ~XMediaEncode();

	//编码器上下文
	AVCodecContext *video_codec_ctx = 0;//视频编码器上下文
	AVCodecContext *audio_codec_ctx = 0;//音频编码器上下文

protected:
    XMediaEncode();

};