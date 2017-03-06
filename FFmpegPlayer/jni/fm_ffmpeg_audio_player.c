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



JNIEXPORT void JNICALL Java_com_fmtech_ffmpeg_FMPlayer_sound
  (JNIEnv *env, jobject jclass, jstring input_jstr, jstring out_jstr){

	const char* input_path = (*env)->GetStringUTFChars(env, input_jstr, NULL);
	const char* output_path = (*env)->GetStringUTFChars(env, out_jstr, NULL);

	//1.注册组件
	av_register_all();

	//2.打开输入音频文件
	//封装格式上下文
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	if(avformat_open_input(&pFormatCtx, input_path, NULL, NULL) != 0){
		LOGE("%s", "打开输入音频文件失败");
		return;
	}

	//获取输入文件信息
	if(avformat_find_stream_info(pFormatCtx, NULL) < 0){
		LOGI("%s", "无法获取输入文件信息");
		return;
	}

	//获取音频流索引位置
	int i=0, audio_stream_index = -1;
	for(; i< pFormatCtx->nb_streams; i++){
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			audio_stream_index = i;
			break;
		}
	}

	//获取解码器
	AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_index]->codec;
	AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);



	(*env)->ReleaseStringUTFChars(env, input_jstr, input_path);
	(*env)->ReleaseStringUTFChars(env, out_jstr, output_path);


}

