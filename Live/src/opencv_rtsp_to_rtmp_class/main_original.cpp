#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <iostream>
extern "C"
{
//#include <libswscale/swscale.h>
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
}

#include "XMediaEncode.h"
#include "XRtmp.h"
//预处理失败，找不到头文件；编译失败，语法错误；链接失败，找不到lib文件；执行失败，找不到动态链接库。
using namespace cv;
using namespace std;

#pragma comment(lib, "opencv_world320d.lib")
//#pragma comment(lib, "swscale.lib")
//#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
//#pragma comment(lib, "avformat.lib")

int _main(int argc, char **argv)
{
	const char *inUrl = "rtsp://admin:ASZSCS@192.168.3.3";
	const char *outUrl = "rtmp://106.14.33.215:1935/live/fmlive";

	XMediaEncode *xMediaEncode = XMediaEncode::Get(0);

	//封装和推流对象
	XRtmp *xRtmp = XRtmp::Get(0);

	Mat frame;
	namedWindow("video");

	//像素格式转换上下文
	//SwsContext *vsc = NULL;

	//输出的数据结构
	//AVFrame *yuv = NULL;

	//注册所有的编解码器;
	//avcodec_register_all();

	//注册所有的封装器
	//av_register_all();

	//注册所有网络协议
	//avformat_network_init();

	//编码器上下文
	//AVCodecContext *codec_ctx = NULL;

	//rtmp flv封装器
	//AVFormatContext *ofmt_ctx = NULL;

	VideoCapture cam;

	int ret = 0;
	try{
		////////////////////////////////////////////////////////////
		///1.使用opencv打开rtsp相机

		cam.open(inUrl);
		//cam.open(0);
		if (!cam.isOpened()){
			throw exception("Cam open failed!");
		}
		cout << inUrl << " cam open success!" << endl;


		int srcW = cam.get(CAP_PROP_FRAME_WIDTH);
		int srcH = cam.get(CAP_PROP_FRAME_HEIGHT);
		int dstW = srcW, dstH = srcH;
		//int fps = cam.get(CAP_PROP_FPS);
		int fps = 25;

		///2.初始化格式转换上下文
		///3.初始化输出的数据结构
		xMediaEncode->inWidth = srcW;
		xMediaEncode->inHeight = srcH;
		xMediaEncode->outWidth = dstW;
		xMediaEncode->outHeight = dstH;
		xMediaEncode->InitScale();


		//vsc = sws_getCachedContext(vsc,
		//	srcW, srcH, AV_PIX_FMT_BGR24,//源宽、高、像素格式
		//	dstW, dstH, AV_PIX_FMT_YUV420P,//目标宽、高、像素格式
		//	SWS_BICUBIC,//尺寸变化使用算法
		//	0, 0, 0);


		/*if (!vsc){
			throw exception("sws_getCachedContext failed.");
		}*/

		///3.初始化输出的数据结构
		/*yuv = av_frame_alloc();
		yuv->format = AV_PIX_FMT_YUV420P;
		yuv->width = srcW;
		yuv->height = srcH;
		yuv->pts = 0;*/

		//分配yuv空间
		/*int ret = av_frame_get_buffer(yuv, 32);
		if (ret != 0){
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf)-1);
			throw exception(buf);
		}*/

		///4.初始化编码上下文
		if (!xMediaEncode->InitVideoCodec()){
			throw exception("InitVideoCodec failed!");
		}
		//a.找到编码器
		/*AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);

		if (!codec){
			throw exception("Can't find h264 encoder!");
		}*/

		xRtmp->Init(outUrl);

		//b.创建编码器上下文
		/*codec_ctx = avcodec_alloc_context3(codec);
		if (!codec_ctx){
			throw exception("avcodec_alloc_context3 failed!");
		}*/

		//c.配置编码器参数
		//codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;//全局参数
		//codec_ctx->codec_id = codec->id;
		//codec_ctx->thread_count = 8;//编码线程数量
		//codec_ctx->bit_rate = 50 * 1024 * 8;//压缩后每秒视频的bit位大小,通过压缩率控制视频的码率；50KB
		//codec_ctx->width = srcW;
		//codec_ctx->height = srcH;
		//codec_ctx->time_base = { 1, fps };//pts以什么数进行计算
		//codec_ctx->framerate = { fps, 1 };//帧率
		//codec_ctx->gop_size = 50;//画面组的大小，多少帧一个关键帧
		//codec_ctx->max_b_frames = 0; //B帧
		//codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;


		////d.打开编码器上下文
		//ret = avcodec_open2(codec_ctx, NULL, NULL);
		//if (ret != 0){
		//	char buf[1024] = { 0 };
		//	av_strerror(ret, buf, sizeof(buf)-1);
		//	throw exception(buf);
		//}
		//cout << "avcodec_open2 success" << endl;

		///5.封装器和视频流配置
		//a.创建输出封装器上下文
		/*ret = avformat_alloc_output_context2(&ofmt_ctx, 0, "flv", outUrl);
		if (ret != 0){
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf)-1);
			throw exception(buf);
		}*/
		//b.添加视频流
		/*AVStream *vstream = avformat_new_stream(ofmt_ctx, NULL);
		if (!vstream){
			throw exception("avformat_new_stream failed");
		}

		vstream->codecpar->codec_tag = 0;*/

		//从编码器复制参数
		/*avcodec_parameters_from_context(vstream->codecpar, xMediaEncode->codec_ctx);
		av_dump_format(ofmt_ctx, 0, outUrl, 1);*/

		xRtmp->AddStream(xMediaEncode->codec_ctx);

		//打开rtmp的网络输出IO
		/*ret = avio_open(&ofmt_ctx->pb, outUrl, AVIO_FLAG_WRITE);
		if (ret != 0){
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf)-1);
			throw exception(buf);
		}*/

		//写入封装头
		//注意：写入封装头后可能会把vstream的time_base改掉，
		//所以等用到time_base的时候一定要，拿视频流中的time_base再进行计算
		/*ret = avformat_write_header(ofmt_ctx, NULL);
		if (ret != 0){
			char buf[1024] = { 0 };
			av_strerror(ret, buf, sizeof(buf)-1);
			throw exception(buf);
		}*/

		xRtmp->SendHeader();

		/*AVPacket pkt;
		memset(&pkt, 0, sizeof(pkt));*/

		int vpts = 0;
		for (;;){
			//读取rtsp视频帧，解码视频帧
			if (!cam.grab()){//只做解码
				continue;
			}

			//yuv转换为rgb
			if (!cam.retrieve(frame)){
				continue;
			}

			imshow("video", frame);
			waitKey(1);


			//rgb to yuv
			xMediaEncode->inPixSize = frame.elemSize();
			AVFrame* yuv = xMediaEncode->RGBToYUV((char*)frame.data);
			if (!yuv){
				continue;
			}
			//输入的数据结构
			//uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
			////bgrbgrbgr
			////plane indata[0]:bbbbb indata[1]:ggggg indata[2]:rrrrr
			//indata[0] = frame.data;
			//int insize[AV_NUM_DATA_POINTERS] = { 0 };
			////一行(宽)数据的字节数
			//insize[0] = frame.cols * frame.elemSize();

			//int h = sws_scale(vsc, indata, insize, 0, frame.rows, //源数据
			//	yuv->data, yuv->linesize);

			//if (h <= 0){
			//	continue;
			//}
			//cout << h << " " << flush;

			///h264编码
			AVPacket *pkt = xMediaEncode->EncodeVideo(yuv);
			if(!pkt){
				continue;
			}

			/*yuv->pts = vpts;
			vpts++;

			ret = avcodec_send_frame(codec_ctx, yuv);
			if (ret != 0){
				continue;
			}

			ret = avcodec_receive_packet(codec_ctx, &pkt);

			if (ret == 0 || pkt.size > 0){
				cout << "*" << pkt.size << flush;
			}
			else{
				continue;
			}*/

			///推流
			/*pkt->pts = av_rescale_q(pkt->pts, xMediaEncode->codec_ctx->time_base, vstream->time_base);
			pkt->dts = av_rescale_q(pkt->dts, xMediaEncode->codec_ctx->time_base, vstream->time_base);

			ret = av_interleaved_write_frame(ofmt_ctx, pkt);

			if (ret == 0){
				cout << "#" << flush;
			}*/
			xRtmp->SendFrame(pkt);
		}


	}
	catch (exception &ex){
		if (cam.isOpened()){
			cam.release();
		}

		/*if (vsc){
			sws_freeContext(vsc);
			vsc = NULL;
		}*/

		/*if (codec_ctx){
			avcodec_free_context(&codec_ctx);
		}*/

		/*if (codec_ctx){
			avio_closep(&ofmt_ctx->pb);
			avcodec_free_context(&codec_ctx);
		}*/

		cerr << ex.what() << endl;
	}



	return 0;
}