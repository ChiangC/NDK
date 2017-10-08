#include "XMediaEncode.h"
extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")

#include <iostream>
using namespace std;

#if defined WIN32 || defined _WIN32
#include <windows.h>
#endif

static int XGetCpuNum()
{
#if defined WIN32 || defined _WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	return (int)sysinfo.dwNumberOfProcessors;

#elif defined __linux__
	return (int)sysconf(SC_NPROCESSORS_ONLN);
#elif defined __APPLE__
	int numCPU = 0;
	int mib[4];
	size_t len = sizeof(numCPU);

	//set the mib for hw.nepu
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU; //alternatively, try HW_NCPU;

	//get the number of CUPUs from the system
	syctl(mib, 2, &numCPU, &len, NULL, 0);

	if (numCPU < 1)
	{
		mib[1] = HW_NCPU;
		sysctl(mib, 2, &numCPU, &len, NULL, 0);

		if (numCPU)
			numCPU = 1;
	}

	return (int)numCPU;
#else
	return 1;
#endif
}



class CXMediaEncode:public XMediaEncode
{
public :
   void Close()
   {

		if (vsc){
			sws_freeContext(vsc);
			vsc = NULL;
		}

        if(yuv){
            av_frame_free(&yuv);
        }

		if (aswr_ctx){
			swr_free(&aswr_ctx);
		}

		if (pcm){
			av_frame_free(&pcm);

		}

		if (video_codec_ctx){
			avcodec_free_context(&video_codec_ctx);
		}

		vpts = 0;
		av_packet_unref(&vpacket);
		av_packet_unref(&apacket);
   }

   bool InitScale()
   {
        vsc = sws_getCachedContext(vsc,
			inWidth, inHeight, AV_PIX_FMT_BGR24,//源宽、高、像素格式
			outWidth, outHeight, AV_PIX_FMT_YUV420P,//目标宽、高、像素格式
			SWS_BICUBIC,//尺寸变化使用算法
			0, 0, 0);
		if(!vsc)
		{
		    cout<<"sws_getCachedContext failed!"<<endl;
		    return false;
		}

		yuv = av_frame_alloc();
        yuv->format = AV_PIX_FMT_YUV420P;
        yuv->width = inWidth;
        yuv->height = inHeight;
        yuv->pts = 0;

        //分配yuv空间
        int ret = av_frame_get_buffer(yuv, 32);
        if (ret != 0){
            return false;
        }

        return true;
   }


    XData RGBToYUV(XData d)
    {
		XData dResult;
        //rgb to yuv
        //输入的数据结构
        uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
        //bgrbgrbgr
        //plane indata[0]:bbbbb indata[1]:ggggg indata[2]:rrrrr
        indata[0] = (uint8_t *)d.data;
        int insize[AV_NUM_DATA_POINTERS] = { 0 };
        //一行(宽)数据的字节数
        insize[0] = inWidth * inPixSize;

        int h = sws_scale(vsc, indata, insize, 0, inHeight, //源数据
            yuv->data, yuv->linesize);

        if (h <= 0){
            return dResult;
        }
		dResult.data = (char*)yuv;
		int *p = yuv->linesize;
		yuv->pts = d.pts;
		dResult.pts = d.pts;
		while ((*p))
		{
			dResult.size += (*p)*outHeight;
			p++;
		}
        return dResult;
    }

    bool InitVideoCodec()
    {
		///4.初始化编码上下文
		////a.找到编码器
		//AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);

		//if (!codec){
		//	cout<<"Can't find h264 encoder!"<<endl;
		//	return false;
		//}

		////b.创建编码器上下文
		//video_codec_ctx = avcodec_alloc_context3(codec);
		//if (!video_codec_ctx){
		//	cout<<"avcodec_alloc_context3 failed!"<<endl;
		//	return false;
		//}
		if (!(video_codec_ctx = CreateCodec(AV_CODEC_ID_H264)))
		{
			return false;
		}

		//c.配置编码器参数
		//video_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;//全局参数
		//video_codec_ctx->codec_id = codec->id;
		//video_codec_ctx->thread_count = XGetCpuNum();//编码线程数量
		video_codec_ctx->bit_rate = 50 * 1024 * 8;//压缩后每秒视频的bit位大小,通过压缩率控制视频的码率；50KB
		video_codec_ctx->width = outWidth;
		video_codec_ctx->height = outHeight;
		//video_codec_ctx->time_base = { 1, fps };//pts以什么数进行计算
		video_codec_ctx->framerate = { fps, 1 };//帧率
		video_codec_ctx->gop_size = 50;//画面组的大小，多少帧一个关键帧
		video_codec_ctx->max_b_frames = 0; //B帧
		video_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

		return OpenCodec(&video_codec_ctx);
    }

	bool InitAudioCodec()
	{
		///4.Init audio codec context
		if (!(audio_codec_ctx = CreateCodec(AV_CODEC_ID_AAC)))
		{
			return false;
		}
		
		audio_codec_ctx->bit_rate = 40000;
		audio_codec_ctx->sample_rate = sampleRate;
		audio_codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
		audio_codec_ctx->channels = channels;
		audio_codec_ctx->channel_layout = av_get_default_channel_layout(channels);
		
		return OpenCodec(&audio_codec_ctx);
	}

	XData EncodeVideo(XData frame)
	{
		XData r;

		av_packet_unref(&vpacket);

		if (frame.size <= 0 || !frame.data)return r;

		///h264编码
		/*yuv->pts = vpts;
		vpts++;*/

		int ret = avcodec_send_frame(video_codec_ctx, (AVFrame*)frame.data);
		if (ret != 0){
			return r;
		}

		ret = avcodec_receive_packet(video_codec_ctx, &vpacket);
		if (ret != 0 || vpacket.size <=0){
			return r;
		}
		r.data = (char*)&vpacket;
		r.size = vpacket.size;
		r.pts = frame.pts;
		return r;
	}

	long long lasta = -1;
	XData EncodeAudio(XData d)
	{
		XData r;
		if (d.size <= 0 || !d.data)
		{
			return r;
		}

		AVFrame *frame = (AVFrame*)d.data;
		if (lasta == frame->pts)
		{
			frame->pts += 1200;//至少大于1000，否则，会因为精度省略掉
		}
		lasta = frame->pts;

		//frame->pts = av_rescale_q(frame->pts, { 1, 1000000 }, audio_codec_ctx->time_base);

		int ret = avcodec_send_frame(audio_codec_ctx, frame);
		if (ret != 0)return r;

		av_packet_unref(&apacket);
		ret = avcodec_receive_packet(audio_codec_ctx, &apacket);
		if (ret != 0){ return r; }
		/*cout << a_packet.size << " " << flush;*/
		r.data = (char*)&apacket;
		r.size = apacket.size;
		r.pts = d.pts;
		return r;
	}

	//**********************************************************//
	bool InitResample()
	{
		//音频重采样，上下文初始化
		
		aswr_ctx = swr_alloc_set_opts(aswr_ctx,
			av_get_default_channel_layout(channels), (AVSampleFormat)outSampleFmt, sampleRate,//输出格式
			av_get_default_channel_layout(channels), (AVSampleFormat)inSampleFmt, sampleRate,//输入格式
			0, 0);

		if (!aswr_ctx)
		{
			cout << "swr_alloc_set_opts failed!" << endl;
		}

		int ret = swr_init(aswr_ctx);
		if (ret != 0)
		{
			char err[1024] = { 0 };
			av_strerror(ret, err, sizeof(err)-1);
			cout << err << endl;
			return false;
		}
		cout << "Audio resample context init success!" << endl;

		pcm = av_frame_alloc();
		pcm->format = (AVSampleFormat)outSampleFmt;
		pcm->channels = channels;
		pcm->channel_layout = av_get_default_channel_layout(channels);
		pcm->nb_samples = nbSamples;//一帧音频，一通道的采样数量
		ret = av_frame_get_buffer(pcm, 0);//给pcm分配存储空间
		if (ret != 0)
		{
			char err[1024] = { 0 };
			av_strerror(ret, err, sizeof(err)-1);
			cout << err << endl;
			return false;
		}
		return true;
	}

	XData Resample(XData xData)
	{
		XData dResult;
		//重采样源数据
		const uint8_t *indata[AV_NUM_DATA_POINTERS] = {0};
		indata[0] = (uint8_t *)xData.data;
		int len = swr_convert(aswr_ctx, pcm->data, pcm->nb_samples,//输出参数，输出存储地址和样本数量
			indata, pcm->nb_samples);
		if (len <= 0)
		{
			return dResult;
		}
		pcm->pts = xData.pts;
		dResult.data = (char*)pcm;
		dResult.size = pcm->nb_samples*pcm->channels * 2;
		dResult.pts = xData.pts;
		return dResult;
	}

private:
	AVCodecContext* CreateCodec(AVCodecID id)
	{
		AVCodec *codec = avcodec_find_encoder(id);
		if (!codec)
		{
			cout << "avcodec_find_encoder failed!" << endl;
			return NULL;
		}

		AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
		if (!codec_ctx)
		{
			cout << "avcodec_alloc_context3 failed!" << endl;
			return NULL;
		}
		cout << "avcode_alloc_context3 success!" << endl;

		codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		codec_ctx->thread_count = XGetCpuNum();
		codec_ctx->time_base = { 1, 1000000 };

		return codec_ctx;
	}

	bool OpenCodec(AVCodecContext **codec_ctx)
	{
		//打开音频编码器
		int ret = avcodec_open2(*codec_ctx, NULL, NULL);
		if (ret != 0)
		{
			char err[1024] = { 0 };
			av_strerror(ret, err, sizeof(err)-1);
			cout << err << endl;
			avcodec_free_context(codec_ctx);
			return false;
		}
		return true;
	}

    SwsContext *vsc = NULL;//像素格式转换上下文
	SwrContext *aswr_ctx = NULL;
	AVFrame *yuv = NULL;
	AVFrame *pcm = NULL;

	AVPacket vpacket;//视频帧
	AVPacket apacket;//音频帧

	int vpts = 0;
	int apts = 0;
};

XMediaEncode * XMediaEncode::Get(unsigned char index)
{
    static bool isFirst  = true;

    if(isFirst)
    {
        avcodec_register_all();
        isFirst = false;
    }

	static CXMediaEncode cxm[255];
    return &cxm[index];
}

XMediaEncode::XMediaEncode()
{

}

XMediaEncode::~XMediaEncode()
{

}