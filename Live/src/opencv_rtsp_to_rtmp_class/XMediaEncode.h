#pragma once

struct AVFrame;
struct AVPacket;
class AVCodecContext;

//音视频编码接口类
class XMediaEncode
{

public :

    int inWidth = 1280;
    int inHeight = 720;
    int inPixSize = 3;

    int outWidth = 1280;
    int outHeight = 720;
    int bitRate = 400000;//压缩后每秒视频的bit位大小,通过压缩率控制视频的码率；
	int fps = 25;

    static XMediaEncode * Get(unsigned char index = 0);

    virtual AVFrame* RGBToYUV(char *rgb) = 0;

    virtual bool InitScale() = 0;

    virtual bool InitVideoCodec() = 0;

	virtual AVPacket* EncodeVideo(AVFrame* frame) = 0;

    virtual ~XMediaEncode();

	//编码器上下文
	AVCodecContext *codec_ctx = 0;

protected:
    XMediaEncode();

};