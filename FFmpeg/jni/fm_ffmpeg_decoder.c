#include "com_fmtech_ffmpeg_VideoUtils.h"

#include <stdio.h>
#include "android/log.h"

#include "libavformat/avformat.h"//封装格式
#include "libavcodec/avcodec.h"//解码
#include "libswscale/swscale.h"//缩放,像素处理


#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"fmtech_ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"fmtech_ffmpeg",FORMAT,##__VA_ARGS__);

JNIEXPORT void JNICALL Java_com_fmtech_ffmpeg_VideoUtils_decode(JNIEnv * env, jclass jcls, jstring input_jstr, jstring output_jstr){
	const char* input_path = (*env)->GetStringUTFChars(env, input_jstr, NULL);
	const char* output_path = (*env)->GetStringUTFChars(env, output_jstr, NULL);

	//1.注册组件
	av_register_all();

	//2.打开输入视频文件
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
	av_init_packet(packet);

	//像素数据(解码数据)
	////内存分配
	AVFrame *pFrame = av_frame_alloc();
	//YUV420
	AVFrame *pFrameYUV = av_frame_alloc();

	//只有指定了AVFrame的像素格式、画面大小才能真正分配内存
	//缓冲区分配内存
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	//初始化缓冲区
//	avpicture_fill(AVPicture *picture, const uint8_t *ptr,enum AVPixelFormat pix_fmt, int width, int height);
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);



	//用于转码（缩放）的参数，转之前的宽高，转之后的宽高，格式等
	struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
			pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
			SWS_BICUBIC, NULL, NULL, NULL);

	//6.一帧一帧读取压缩的视频数据AVPacket
	//输出文件
	FILE* fp_yuv = fopen(output_path, "wb");

	int got_picture;
	int len;
	int frame_count = 0;
	while(av_read_frame(pFormatCtx, packet) >= 0){
		//7.解码一帧视频压缩数据，得到视频像素数据
		//解码AVPacket===>AVFrame
		len = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
		//0 zero if no frame could be decompressed
		//非零，正在解码
		if(got_picture){
			//frame--->yuvFrame (YUV420P)
			//转为指定的YUV420P像素帧
			//2 6输入、输出数据
			//3 7输入、输出画面一行的数据的大小 AVFrame 转换是一行一行转换的
			//4 输入数据第一列要转码的位置 从0开始
			//5 输入画面的高度
			sws_scale(sws_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameYUV->data, pFrameYUV->linesize);

			/*int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
			              const int srcStride[], int srcSliceY, int srcSliceH,
			              uint8_t *const dst[], const int dstStride[]);*/

			//向YUV文件保存解码之后的帧数据
			//AVFrame--->YUV
			//一个像素包含一个Y
			int y_size = pCodecCtx->width * pCodecCtx->height;
			fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);
			fwrite(pFrameYUV->data[1], 1, y_size/4, fp_yuv);
			fwrite(pFrameYUV->data[2], 1, y_size/4, fp_yuv);
			frame_count++;
			LOGI("解码第%d帧", frame_count);
			printf("解码第%d帧", frame_count);
		}

		av_free_packet(packet);
	}

	fclose(fp_yuv);
	av_frame_free(&pFrame);
	av_frame_free(&pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_free_context(pFormatCtx);

	(*env)->ReleaseStringUTFChars(env, input_jstr, input_path);
	(*env)->ReleaseStringUTFChars(env, output_jstr, output_path);


}

