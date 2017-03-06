#include "com_fmtech_ffmpeg_FMPlayer.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include "libyuv.h"
#include "libavformat/avformat.h"//封装格式
#include "libavcodec/avcodec.h"//解码
#include "libswscale/swscale.h"//缩放,像素处理


#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"fmtech_ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"fmtech_ffmpeg",FORMAT,##__VA_ARGS__);



JNIEXPORT void JNICALL Java_com_fmtech_ffmpeg_FMPlayer_render
  (JNIEnv *env, jobject jclass, jstring input_jstr, jobject surface){
	const char* input_path = (*env)->GetStringUTFChars(env, input_jstr, NULL);

	//1.注册组件
	av_register_all();

	//2.打开输入视频文件
	//封装格式上下文
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	if(avformat_open_input(&pFormatCtx, input_path, NULL, NULL) != 0){
		LOGE("%s", "打开输入视频文件失败");
		return;
	}

	//3.获取视频信息
	if(avformat_find_stream_info(pFormatCtx, NULL) < 0){
		LOGE("%s", "获取视频信息失败");
		return;
	}

	//视频解码，需要找到视频对应的AVStream所在pFormatCtx->streams的索引位置
	int video_stream_index = -1;
	int i = 0;
	for(; i < pFormatCtx->nb_streams; i++){
		//根据类型判断，是否是视频流
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			video_stream_index = i;
			break;
		}
	}

	//4.获取视频解码器
	AVCodecContext *pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;
	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec == NULL){
		LOGE("%s", "无法解码");
		return;
	}

	//5.打开解码器
	if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
		LOGE("%s", "解码器无法打开");
		return;
	}

	//编码数据
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
//	av_init_packet(packet);

	//像素数据(解码数据)
	////内存分配
	AVFrame *pRGB_Frame = av_frame_alloc();
	//YUV420
	AVFrame *pYUV_Frame = av_frame_alloc();

	//native绘制
	//窗体
	ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);
	//绘制时的缓冲区
	ANativeWindow_Buffer outBuffer;

	int got_picture;
	int len;
	int frame_count = 0;
	while(av_read_frame(pFormatCtx, packet) >= 0){
		//7.解码一帧视频压缩数据，得到视频像素数据
		//解码AVPacket===>AVFrame
		len = avcodec_decode_video2(pCodecCtx, pYUV_Frame, &got_picture, packet);
		//0 zero if no frame could be decompressed
		//非零，正在解码
		if(got_picture){
			//lock
			//设置缓冲区的属性，宽高，像素格式
			ANativeWindow_setBuffersGeometry(nativeWindow, pCodecCtx->width, pCodecCtx->height, WINDOW_FORMAT_RGBA_8888);
			ANativeWindow_lock(nativeWindow, &outBuffer, NULL);

			//设置RGB Frame的属性(宽高，像素格式)和缓冲区
			//rgb缓冲区与outBuffer.bits是同一块内存
			avpicture_fill((AVPicture *)pRGB_Frame, outBuffer.bits, PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height);

			//YUV->RGBA_8888
			I420ToARGB(pYUV_Frame->data[0], pYUV_Frame->linesize[0],
					pYUV_Frame->data[2], pYUV_Frame->linesize[2],
					pYUV_Frame->data[1], pYUV_Frame->linesize[1],
					pRGB_Frame->data[0], pRGB_Frame->linesize[0],
					pCodecCtx->width, pCodecCtx->height);
			//fix buffer


			//unlock
			ANativeWindow_unlockAndPost(nativeWindow);

			//sleep 一下
			usleep(1000*16);//16ms
		}

		av_free_packet(packet);
	}


	ANativeWindow_release(nativeWindow);
	av_frame_free(&pRGB_Frame);
	av_frame_free(&pYUV_Frame);
	avcodec_close(pCodecCtx);
	avformat_free_context(pFormatCtx);

	(*env)->ReleaseStringUTFChars(env, input_jstr, input_path);


}

