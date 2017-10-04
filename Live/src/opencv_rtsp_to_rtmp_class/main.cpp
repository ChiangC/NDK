//预处理失败，找不到头文件；
//编译失败，语法错误；
//链接失败，找不到lib文件；
//执行失败，找不到动态链接库

/***********************************************
1.使用opencv打开rtsp相机

2.初始化格式转换上下文

3.初始化输出的数据结构

4.初始化编码上下文

5.封装器和视频流配置

6.h264编码

7.推流
***********************************************/


#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <iostream>

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#pragma comment(lib, "opencv_world320d.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avformat.lib")

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	const char *inUrl = "rtsp://admin:ASZSCS@192.168.3.3";
	const char *outUrl = "rtmp://106.14.33.215:1935/live/fmlive";

	//像素格式转换上下文
	SwsContext *sws_ctx = NULL;

	//编码器上下文
	AVCodecContext *codec_ctx = NULL;

	//rtmp flv封装器
	AVFormatContext *ofmt_ctx = NULL;

	//输出的数据结构
	AVFrame *yuv_frame = NULL;

	Mat frame;
	namedWindow("Opencv Video");
	VideoCapture camera;

	//注册所有的编解码器
	avcodec_register_all();

	//注册所有的封装器
	av_register_all();

	//注册所有网络协议
	avformat_network_init();

	try{
		///1.使用opencv打开rtsp相机
		camera.open(inUrl);
		//camera.open(0);
		if (!camera.isOpened()){
			throw exception("Camera open failed!");
		}

		cout << inUrl << "--camera open success!" << endl;
		int srcW = camera.get(CAP_PROP_FRAME_WIDTH);
		int srcH = camera.get(CAP_PROP_FRAME_HEIGHT);
		int dstW = srcW, dstH = srcH;
		int fps = 25;//fps = camera.get(CAP_PROP_FPS);

		///2.初始化格式转换上下文
		sws_ctx = sws_getCachedContext(sws_ctx,
			srcW, srcH, AV_PIX_FMT_BGR24,//源宽、高、像素格式
			dstW, dstH, AV_PIX_FMT_YUV420P,//目标宽、高，像素格式
			SWS_BICUBIC,//尺寸变化使用算法
			0, 0, 0);

		if(!sws_ctx)
		{
			throw exception("sws_getCachedContext failed.");
		}


		///3.初始化输出的数据结构
		yuv_frame = av_frame_alloc();
		yuv_frame->format = AV_PIX_FMT_YUV420P;
		yuv_frame->width = srcW;
		yuv_frame->height = srcH;
		yuv_frame->pts = 0;
		//分配yuv空间
		int ret = av_frame_get_buffer(yuv_frame, 32);
		if (ret != 0)
		{
			char buf[1024] = {0};
			av_strerror(ret, buf, sizeof(buf) - 1);
			throw exception(buf);
		}


		///4.初始化编码上下文
		//a.找到编码器
		AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		if (!codec)
		{
			throw exception("Can't find h264 encoder!");
		}
		//b.创建编码器上下文
		codec_ctx = avcodec_alloc_context3(codec);
		if (!codec_ctx)
		{
			throw exception("avcodec_alloc_context3 failed!");
		}
		//c.配置编码器参数
		codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;//全局参数
		codec_ctx->codec_id = codec->id;
		codec_ctx->thread_count = 8;//编码线程数
		codec_ctx->bit_rate = 50 * 1024 * 8;//50KB;压缩后每秒视频的bit位大小，通过压缩率控制视频的码率;
		codec_ctx->width = srcW;
		codec_ctx->height = srcH;
		codec_ctx->time_base = {1, fps};//pts以什么数进行计算
		codec_ctx->framerate = { fps, 1 };//帧率
		codec_ctx->gop_size = 50;//画面组的大小，多少帧一个关键帧；设置得越大，同等质量的画面压缩率越高
		codec_ctx->max_b_frames = 0;//B帧；B帧为0，则pts和dts一致。
		codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
		
		//d.打开编码器上下文
		ret = avcodec_open2(codec_ctx, NULL, NULL);
		if (ret != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			throw exception(buf);
		}
		cout << "avcodec_open2 success" << endl;
		
		
		///5.封装器和视频流配置
		//a.创建输出封装器上下文
		//这个函数的format_name参数，如果是输出文件可以不传，因为是输出流所以要传(流媒体判断不出来)
		ret = avformat_alloc_output_context2(&ofmt_ctx, 0, "flv", outUrl);
		if (ret != 0)
		{
			char buf[1024] = {0};
			av_strerror(ret, buf, sizeof(buf) - 1);
			throw exception(buf);
		}
		//b.添加视频流
		AVStream *vstream = avformat_new_stream(ofmt_ctx, NULL);
		if (!vstream)
		{
			throw exception("avformat_new_stream failed");
		}
		vstream->codecpar->codec_tag = 0;//指定编码格式不要进行设置
		//c.从编码器复制参数
		avcodec_parameters_from_context(vstream->codecpar, codec_ctx);
		av_dump_format(ofmt_ctx, 0, outUrl, 1);
		//d.打开rtmp的网络输出IO
		ret = avio_open(&ofmt_ctx->pb, outUrl, AVIO_FLAG_WRITE);
		if (ret != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf) - 1);
			throw exception(buf);
		}
		//e.写入封装头
		//注意：写入封装头后可能会把vstream的time_base改掉
		//所以，等用到time_base的时候，一定要拿视频流中的time_base再进行计算
		ret = avformat_write_header(ofmt_ctx, NULL);
		if (ret != 0)
		{
			char buf[1024] = {0};
			av_strerror(ret, buf, sizeof(buf) - 1);
			throw exception(buf);
		}

		//6.rgb to yuv
		AVPacket pkt;
		memset(&pkt, 0, sizeof(pkt));
		int vpts = 0;

		for (;;)
		{
			//读取rtsp视频帧，解码视频帧
			if (!camera.grab())//只做解码
			{
				continue;
			}

			//yuv转为rgb
			if (!camera.retrieve(frame))
			{
				continue;
			}

			imshow("Opencv Video", frame);
			waitKey(1);


			//rgb to yuv
			//输入的数据结构
			uint8_t *indata[AV_NUM_DATA_POINTERS] = {0};
			//bgrbgrbgr
			//plane indata[0]:bbbbb indata[1]:ggggg indata[]:rrrrr
			indata[0] = frame.data;//bgrbgrbgr在这里是这种方式
			int insize[AV_NUM_DATA_POINTERS] = {0};
			insize[0] = frame.cols * frame.elemSize();//一行(宽)数据的字节数
			int h = sws_scale(sws_ctx, indata, insize, 0, frame.rows,//源数据
				yuv_frame->data, yuv_frame->linesize);
			if (h <= 0)
			{
				continue;
			}
			cout << h << " " << flush;


			///7.h264编码
			yuv_frame->pts = vpts;
			vpts++;
			//内部使用多线程进行编码
			ret = avcodec_send_frame(codec_ctx, yuv_frame);
			if (ret != 0)
			{
				continue;
			}
			ret = avcodec_receive_packet(codec_ctx, &pkt);
			if (ret != 0 || pkt.size > 0)
			{
				cout << "*" << pkt.size << flush;
			}else{
				continue;
			}
			
			///8.推流
			//把pkt.pts由基于codec_ctx->time_base转换为基于vstream->time_base的pts.
			pkt.pts = av_rescale_q(pkt.pts, codec_ctx->time_base, vstream->time_base);
			pkt.dts = av_rescale_q(pkt.dts, codec_ctx->time_base, vstream->time_base);
			//av_write_frame(ofmt_ctx, &pkt);这个函数不进行排序
			ret = av_interleaved_write_frame(ofmt_ctx, &pkt);//这个函数的好处：第一、内部有缓冲排序；第二、每次不管成功与否都会释放packet内部的数据；
			if (ret == 0)
			{
				cout << "#" << flush;
			}
		
		}

	}catch (exception &ex){
		if (camera.isOpened())
		{
			camera.release();
		}

		if (sws_ctx)
		{
			sws_freeContext(sws_ctx);
			sws_ctx = NULL;
		}

		if (codec_ctx)
		{
			avcodec_free_context(&codec_ctx);
		}

		if (codec_ctx)
		{
			avio_closep(&ofmt_ctx->pb);
			//或者使用下面的方式：
			/*avio_close(ofmt_ctx->pb);
			ofmt_ctx->pb = NULL;*/
			avcodec_free_context(&codec_ctx);
		}

		cerr << ex.what() << endl;
	}




	getchar();
	return 0;
}
