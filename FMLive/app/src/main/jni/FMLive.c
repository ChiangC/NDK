#include "com_fmtech_fmlive_jni_PushNative.h"
#include <stdint.h>
#include <android/log.h>
#include <pthread.h>
#include "x264.h"
#include "faac.h"
#include "rtmp.h"
#include "queue.h"

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define CONNECT_FAILED 101
#define INIT_FAILED 102

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"FMLive",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"FMLive",FORMAT,##__VA_ARGS__);

//x264��������ͼ��YUV420P
x264_picture_t pic_in;
x264_picture_t pic_out;

//YUV����
int y_len, u_len, v_len;

unsigned int start_time;

//x264���봦����
x264_t *video_encode_handler;

pthread_mutex_t mutex;
pthread_cond_t cond;

//pthread_t push_thread_id;

//rtmp��ý���ַ
char *rtmp_url;

//�Ƿ�ֱ��
int is_pushing = FALSE;

//��Ƶ���봦����
faacEncHandle audio_encode_handle;

unsigned long inputSamples;//�����������
unsigned long maxOutputBytes;//�������֮����ֽ���

jobject jobj_push_native;//Global Ref
jmethodID jmid_throw_native_error;
jclass jcls_push_native;

JavaVM *javaVM;

//��ȡJavaVM
jint JNI_OnLoad(JavaVM* vm, void* reserved){
	javaVM = vm;
	return JNI_VERSION_1_4;//jni1.4����֧��
}

void throwNativeError(JNIEnv *env,int code){
	LOGI("%s", "throwNativeError");
	//��Java�㷢�ʹ�����Ϣ
	(*env)->CallVoidMethod(env, jobj_push_native, jmid_throw_native_error, code);
}

/**
 * ����RTMPPacket���У��ȴ������̷߳���
 */
void add_rtmp_packet(RTMPPacket *packet){
	pthread_mutex_lock(&mutex);

	if(is_pushing){
		queue_append_last(packet);
	}

	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

/**
 * ����h264 SPS��PPS������
 */
void add_264_sequence_header(unsigned char* pps, unsigned char* sps, int pps_len, int sps_len){
//	LOGI("%s","-------add_264_sequence_header");
	int body_size = 16 + sps_len + pps_len;//��װH264��׼����PPS��SPS����ʹ����16���ֽ�
	RTMPPacket *packet = malloc(sizeof(RTMPPacket));
	//RTMPPacket��ʼ��
	RTMPPacket_Alloc(packet,body_size);
	RTMPPacket_Reset(packet);

	unsigned char * body = packet->m_body;
	int i = 0;
	//�����Ʊ�ʾ��00010111
	body[i++] = 0x17;//VideoHeaderTag:FrameType(1=key frame)+CodecID(7=AVC)
	body[i++] = 0x00;//AVCPacketType = 0��ʾ����AVCDecoderConfigurationRecord
	//composition time 0x000000 24bit ?
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	/*AVCDecoderConfigurationRecord*/
	body[i++] = 0x01;//configurationVersion���汾Ϊ1
	body[i++] = sps[1];//AVCProfileIndication
	body[i++] = sps[2];//profile_compatibility
	body[i++] = sps[3];//AVCLevelIndication
	//?
	body[i++] = 0xFF;//lengthSizeMinusOne,H264 ��Ƶ�� NALU�ĳ��ȣ����㷽���� 1 + (lengthSizeMinusOne & 3),ʵ�ʲ���ʱ������ΪFF��������Ϊ4.

	/*sps*/
	body[i++] = 0xE1;//numOfSequenceParameterSets:SPS�ĸ��������㷽���� numOfSequenceParameterSets & 0x1F,ʵ�ʲ���ʱ������ΪE1��������Ϊ1.
	body[i++] = (sps_len >> 8) & 0xff;//sequenceParameterSetLength:SPS�ĳ���
	body[i++] = sps_len & 0xff;//sequenceParameterSetNALUnits
	memcpy(&body[i], sps, sps_len);
	i += sps_len;

	/*pps*/
	body[i++] = 0x01;//numOfPictureParameterSets:PPS �ĸ���,���㷽���� numOfPictureParameterSets & 0x1F,ʵ�ʲ���ʱ������ΪE1��������Ϊ1.
	body[i++] = (pps_len >> 8) & 0xff;//pictureParameterSetLength:PPS�ĳ���
	body[i++] = (pps_len) & 0xff;//PPS
	memcpy(&body[i], pps, pps_len);
	i += pps_len;

	//Message Type��RTMP_PACKET_TYPE_VIDEO��0x09
	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	//Payload Length
	packet->m_nBodySize = body_size;
	//Time Stamp��4�ֽ�
	//��¼��ÿһ��tag����ڵ�һ��tag��File Header�������ʱ�䡣
	//�Ժ���Ϊ��λ����File Header��time stamp��ԶΪ0��
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_nChannel = 0x04; //Channel ID��Audio��Vidioͨ��
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM; //?
	//��RTMPPacket�������
	add_rtmp_packet(packet);
}

/**
 * ����h264֡��Ϣ
 */
void add_264_body(unsigned char *buf, int len){
//	LOGI("%s","-------add_264_body");
	//ȥ����ʼ��(�綨��)
	if(buf[2] == 0x00){//00 00 00 01
		buf += 4;
		len -= 4;
	}else if(buf[2] == 0x01){//00 00 01
		buf += 3;
		len -= 3;
	}
	int body_size = len + 9;
	RTMPPacket *packet = malloc(sizeof(RTMPPacket));
	RTMPPacket_Alloc(packet, body_size);

	unsigned char * body = packet->m_body;
	//��NALͷ��Ϣ�У�type��5λ������5��˵�����ǹؼ�֡NAL��Ԫ
	//buf[0] NAL Header�����㣬��ȡtype������type�жϹؼ�֡����ͨ֡
	//00000101 & 00011111(0x1f) = 00000101
	int type = buf[0] & 0x1f;
	//Inter Frame ֡��ѹ��
	body[0] = 0x27;//VideoHeaderTag:FrameType(2=Inter Frame)+CodecID(7=AVC)
	//IDR I֡ͼ��
	if (type == NAL_SLICE_IDR) {
		body[0] = 0x17;//VideoHeaderTag:FrameType(1=key frame)+CodecID(7=AVC)
	}
	//AVCPacketType = 1
	//0x01������������ɸ�NALU
	body[1] = 0x01; /*nal unit,NALUs��AVCPacketType == 1)*/
	body[2] = 0x00; //composition time 0x000000 24bit
	body[3] = 0x00;
	body[4] = 0x00;

	//д��NALU��Ϣ������8λ��һ���ֽڵĶ�ȡ��
	body[5] = (len >> 24) & 0xff;
	body[6] = (len >> 16) & 0xff;
	body[7] = (len >> 8) & 0xff;
	body[8] = (len) & 0xff;

	/*copy data*/
	memcpy(&body[9], buf, len);

	packet->m_hasAbsTimestamp = 0;
	packet->m_nBodySize = body_size;
	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;//��ǰpacket�����ͣ�Video
	packet->m_nChannel = 0x04;
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
//	packet->m_nTimeStamp = -1;
	packet->m_nTimeStamp = RTMP_GetTime() - start_time;//��¼��ÿһ��tag����ڵ�һ��tag��File Header�������ʱ��
	add_rtmp_packet(packet);
}

/**
 * ���ɼ�������Ƶ���ݽ��б���
 */
JNIEXPORT void JNICALL Java_com_fmtech_fmlive_jni_PushNative_fireVideo
  (JNIEnv *env, jobject jobj, jbyteArray buffer){
	//��Ƶ����תΪYUV420P
	//NV21->YUV420P
	jbyte* nv21_buffer = (*env)->GetByteArrayElements(env, buffer, NULL);
	jbyte* u = pic_in.img.plane[1];
	jbyte* v = pic_in.img.plane[2];
	//nv21 4:2:0 Formats, 12 Bits per Pixel
	//nv21��yuv420p��y����һ�£�uvλ�öԵ�
	//nv21תyuv420p  y = w*h,u/v=w*h/4
	//nv21 = yvu yuv420p=yuv y=y u=y+1+1 v=y+1
	memcpy(pic_in.img.plane[0], nv21_buffer, y_len);
	int i;
	for (i = 0; i < u_len; i++) {
		*(u + i) = *(nv21_buffer + y_len + i * 2 + 1);
		*(v + i) = *(nv21_buffer + y_len + i * 2);
	}

	//h264����֮��õ�NALU����
	x264_nal_t *nal = NULL;//NAL
	int n_nal = -1;//NALU�ĸ���
	if(x264_encoder_encode(video_encode_handler, &nal, &n_nal, &pic_in, &pic_out) < 0){
		LOGE("%s","-------x264_encode failed");
		return;
	}
//	LOGI("%s","-------x264_encode success");
	//ʹ��rtmpЭ�齫h264�������Ƶ���ݷ��͸���ý�������
	//֡��Ϊ�ؼ�֡����ͨ֡��Ϊ�����֡����ľ����ʣ��ؼ�֡Ӧ����SPS��PPS����
	int sps_len, pps_len;
	unsigned char sps[100];
	unsigned char pps[100];
	memset(sps, 0, 100);
	memset(pps, 0, 100);

	pic_in.i_pts += 1;//˳���ۼ�
	//����NALU���飬����NALU�������ж�
	for(i = 0; i < n_nal; i++){
		if(nal[i].i_type == NAL_SPS){
			//����SPS����
			sps_len = nal[i].i_payload - 4;
			//���������ֽ���ʼ��
			memcpy(sps,nal[i].p_payload + 4, sps_len);
		}else if(nal[i].i_type == NAL_PPS){
			//����SPS����
			pps_len = nal[i].i_payload - 4;
			//���������ֽ���ʼ��
			memcpy(pps,nal[i].p_payload + 4, pps_len);

			//����������Ϣ
			//��SPS��PPS������ӵ�h264�ؼ�֡������
			add_264_sequence_header(pps, sps, pps_len, sps_len);
		}else{
			//������ͨ֡
			add_264_body(nal[i].p_payload, nal[i].i_payload);
		}
	}

	(*env)->ReleaseByteArrayElements(env,buffer,nv21_buffer,NULL);

}

/**
 * ���AACͷ��Ϣ
 */
void add_aac_sequence_header(){
//	LOGI("%s","-------add_aac_sequence_header");
	unsigned char *ppBuffer;
	unsigned long len;
	faacEncGetDecoderSpecificInfo(audio_encode_handle, &ppBuffer,&len);
	int body_size = 2 + len;
	RTMPPacket *packet = malloc(sizeof(RTMPPacket));
	//RTMPPacket��ʼ��
	RTMPPacket_Alloc(packet,body_size);
	RTMPPacket_Reset(packet);
	unsigned char *body = packet->m_body;
	//ͷ��Ϣ����
	/*AF 00 + AAC RAW data*/
	body[0] = 0xAF;//10 5 SoundFormat(4bits):10=AAC,SoundRate(2bits):3=44kHz,SoundSize(1bit):1=16-bit samples,SoundType(1bit):1=Stereo sound
	body[1] = 0x00;//AACPacketType:0��ʾAAC sequence header
	memcpy(&body[2], ppBuffer, len); /*spec_buf��AAC sequence header����*/
	packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
	packet->m_nBodySize = body_size;
	packet->m_nChannel = 0x04;
	packet->m_hasAbsTimestamp = 0;
	packet->m_nTimeStamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	add_rtmp_packet(packet);
	free(ppBuffer);

}

/**
 * ���AAC rtmp packet
 */
void add_aac_body(unsigned char *buf, int len){
//	LOGI("%s","-------add_aac_body");
	int body_size = 2 + len;
		RTMPPacket *packet = malloc(sizeof(RTMPPacket));
		//RTMPPacket��ʼ��
		RTMPPacket_Alloc(packet,body_size);
		RTMPPacket_Reset(packet);
		unsigned char * body = packet->m_body;
		//ͷ��Ϣ����
		/*AF 00 + AAC RAW data*/
		body[0] = 0xAF;//10 5 SoundFormat(4bits):10=AAC,SoundRate(2bits):3=44kHz,SoundSize(1bit):1=16-bit samples,SoundType(1bit):1=Stereo sound
		body[1] = 0x01;//AACPacketType:1��ʾAAC raw
		memcpy(&body[2], buf, len); /*spec_buf��AAC raw����*/
		packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
		packet->m_nBodySize = body_size;
		packet->m_nChannel = 0x04;
		packet->m_hasAbsTimestamp = 0;
		packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
		packet->m_nTimeStamp = RTMP_GetTime() - start_time;
		add_rtmp_packet(packet);
}

/*
 * �Բɼ�������Ƶ���ݽ��б���
 * Class:     com_fmtech_fmlive_jni_PushNative
 * Method:    fireAudio
 * Signature: ([BI)V
 */
JNIEXPORT void JNICALL Java_com_fmtech_fmlive_jni_PushNative_fireAudio
  (JNIEnv *env, jobject jobj, jbyteArray buffer, jint len){
	int *pcmbuf;
	unsigned char *bitbuf;
	jbyte* b_buffer = (*env)->GetByteArrayElements(env, buffer, 0);
	pcmbuf = (short*) malloc(inputSamples * sizeof(int));
	bitbuf = (unsigned char*) malloc(maxOutputBytes * sizeof(unsigned char));
	int nByteCount = 0;
	unsigned int nBufferSize = (unsigned int) len / 2;
	unsigned short* buf = (unsigned short*) b_buffer;
	while (nByteCount < nBufferSize) {
		int audioLength = inputSamples;
		if ((nByteCount + inputSamples) >= nBufferSize) {
			audioLength = nBufferSize - nByteCount;
		}
		int i;
		for (i = 0; i < audioLength; i++) {//ÿ�δ�ʵʱ��pcm��Ƶ�����ж�������λ��Ϊ8��pcm���ݡ�
			int s = ((int16_t *) buf + nByteCount)[i];
			pcmbuf[i] = s << 8;//��8��������λ����ʾһ�����������㣨ģ��ת����
		}
		nByteCount += inputSamples;
		//����FAAC���б��룬pcmbufΪת�����pcm�����ݣ�audioLengthΪ����faacEncOpenʱ�õ��������������bitbufΪ����������buff��nMaxOutputBytesΪ����faacEncOpenʱ�õ����������ֽ���
		int byteslen = faacEncEncode(audio_encode_handle, pcmbuf, audioLength,
				bitbuf, maxOutputBytes);
		if (byteslen < 1) {
			continue;
		}
		add_aac_body(bitbuf, byteslen);//��bitbuf�еõ�������aac���������ŵ����ݶ���
	}
	(*env)->ReleaseByteArrayElements(env, buffer, b_buffer, NULL);
	if (bitbuf)
		free(bitbuf);
	if (pcmbuf)
		free(pcmbuf);
}


void *push_action(void* arg){
	JNIEnv* env;
	(*javaVM)->AttachCurrentThread(javaVM,&env,NULL);

	//����RTMP����
	RTMP *rtmp = RTMP_Alloc();
	if(!rtmp){
		LOGE("%s","-------rtmp init failed");
		throwNativeError(env, INIT_FAILED);
		goto end;
	}

	LOGI("%s","-------rtmp init success");
	LOGI("-------rtmp_url:%s",rtmp_url);
	RTMP_Init(rtmp);
	rtmp->Link.timeout = 5;//���ӳ�ʱʱ��10s
	//������ý���ַ
	RTMP_SetupURL(rtmp, rtmp_url);
//	LOGI("%s",rtmp_url);
	//��������rtmp������
	RTMP_EnableWrite(rtmp);

	//��������
	if(!RTMP_Connect(rtmp,NULL)){
		LOGE("%s","-------RTMP_Connect failed");
		throwNativeError(env,CONNECT_FAILED);
		goto end;
	}
	LOGI("%s","-------RTMP_Connect success");

	//��ʱ
	start_time = RTMP_GetTime();
	if(!RTMP_ConnectStream(rtmp,0)){//������
		LOGE("%s","-------RTMP_ConnectStream failed");
		goto end;
	}else{
		LOGI("%s","-------RTMP_ConnectStream success");
	}

	is_pushing = TRUE;

	//����AACͷ��Ϣ,ֻ��Ҫ����һ�ξͿ�����
	add_aac_sequence_header();

	while(is_pushing){
		//����
		pthread_mutex_lock(&mutex);
		//����
		pthread_cond_wait(&cond,&mutex);
		//ȡ�������е�RTMPPacket
		RTMPPacket *packet = queue_get_first();

		if(packet){
			queue_delete_first();//�Ƴ�
			packet->m_nInfoField2 = rtmp->m_stream_id;//RTMPЭ�飬stream_id����
			int result = RTMP_SendPacket(rtmp,packet,TRUE);//TRUE����librtmp�����У���������������
			if(!result){
				LOGE("%s","RTMP disconnected");
				throwNativeError(env,CONNECT_FAILED);
				RTMPPacket_Free(packet);
				pthread_mutex_unlock(&mutex);
				goto end;
			}
//			LOGI("%s","RTMP send packet success");
			RTMPPacket_Free(packet);
		}else{
			LOGI("%s","RTMP get packet failed");
		}

		pthread_mutex_unlock(&mutex);
	}

end:
	LOGI("%s","-------RTMP Release resource");
	if(NULL != rtmp){
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
	}
	(*javaVM)->DetachCurrentThread(javaVM);
	return 0;
}

/*
 * Class:     com_fmtech_fmlive_jni_PushNative
 * Method:    startPush
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_fmtech_fmlive_jni_PushNative_startPush
  (JNIEnv *env, jobject jobj, jstring url_jstr){
	//jobj(PushNative����)
	jobj_push_native = (*env)->NewGlobalRef(env,jobj);

	//PushNative.throwNativeError
	jclass jcls_push_native_tmp = (*env)->GetObjectClass(env, jobj_push_native);
//	jmethodID jmid_throw_native_error = (*env)->GetMethodID(env, jcls_push_native, "throwNativeError", "(I)V");

	jcls_push_native = (*env)->NewGlobalRef(env,jcls_push_native_tmp);
	jmid_throw_native_error = (*env)->GetMethodID(env, jcls_push_native, "throwNativeError", "(I)V");
	if(jmid_throw_native_error == NULL){
		LOGI("%s","-------getMethodID FAILED");
	}else{
		LOGI("%s","-------getMethodID success");
	}
	const char* url_cstr = (*env)->GetStringUTFChars(env, url_jstr, NULL);
	//����url_cstr���ݵ�rtmp_url
	rtmp_url = malloc(strlen(url_cstr)+1);
	memset(rtmp_url,0,strlen(url_cstr)+1);
	memcpy(rtmp_url,url_cstr,strlen(url_cstr));

	//��ʼ������������������
	pthread_mutex_init(&mutex,NULL);
	pthread_cond_init(&cond,NULL);

	//��������
	create_queue();

	//�����������߳�(�Ӷ����в�����ȡRTMPPacket���͸���ý�������)
	pthread_t push_thread_id;
	pthread_create(&push_thread_id, NULL, push_action, NULL);
	(*env)->ReleaseStringUTFChars(env, url_jstr, url_cstr);
}

/*
 * Class:     com_fmtech_fmlive_jni_PushNative
 * Method:    stopPush
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_fmtech_fmlive_jni_PushNative_stopPush
  (JNIEnv *env, jobject jobj){
	is_pushing = FALSE;
//	free(rtmp_url);

}

/*
 * Class:     com_fmtech_fmlive_jni_PushNative
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_fmtech_fmlive_jni_PushNative_release
  (JNIEnv *env, jobject jobj){

	(*env)->DeleteGlobalRef(env, jcls_push_native);
	(*env)->DeleteGlobalRef(env, jobj_push_native);
	(*env)->DeleteGlobalRef(env, jmid_throw_native_error);

}

/*
 * Class:     com_fmtech_fmlive_jni_PushNative
 * Method:    setVideoOptions
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_com_fmtech_fmlive_jni_PushNative_setVideoOptions
  (JNIEnv *env, jobject jobj, jint width, jint height, jint bitrate, jint fps){
	//x264_param_default_preset ����
	x264_param_t param;
	x264_param_default_preset(&param, "ultrafast", "zerolatency");
	//������������ظ�ʽ
	param.i_csp = X264_CSP_I420;
	param.i_width  = width;
	param.i_height = height;

	y_len = width * height;
	u_len = y_len/4;
	v_len = u_len;

	//����i_rc_method��ʾ���ʿ��ƣ�CQP(�㶨����)��CRF(�㶨����)��ABR(ƽ������)
	//�㶨���ʣ��ᾡ�������ڹ̶�����
	param.rc.i_rc_method = X264_RC_CRF;
	param.rc.i_bitrate = bitrate / 1000; //* ����(������,��λKbps)
	param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2; //˲ʱ�������

	param.i_fps_num = fps; //* ֡�ʷ���
	param.i_fps_den = 1; //* ֡�ʷ�ĸ
	param.i_timebase_den = param.i_fps_num;
	param.i_timebase_num = param.i_fps_den;
	param.i_threads = 1;//���б����߳�������0Ĭ��Ϊ���߳�

	/* VFR input.  If 1, use timebase and timestamps for ratecontrol purposes.
	* If 0, use fps only. */
	param.b_vfr_input = 0;

	/* put SPS/PPS before each keyframe */
	//SPS Sequence Parameter Set���в�����PPS Picture Parameter Setͼ�������
	//Ϊ�����ͼ��ľ�������
	param.b_repeat_headers = 1;
	param.b_annexb = 1;
	//����level����
	param.i_level_idc = 51;//5.1
	//x264_param_apply_profile //���õ���
	x264_param_apply_profile(&param, "baseline");

	//����ͼ���ʼ��
	//x264_picture_t pic_in;
	x264_picture_alloc( &pic_in, param.i_csp, param.i_width, param.i_height );

	//�򿪱�����
	video_encode_handler = x264_encoder_open(&param);
	if(video_encode_handler){
		LOGI("%s","-------Open video_encode success");
	}

}

/*
 * ��Ƶ����������
 * Class:     com_fmtech_fmlive_jni_PushNative
 * Method:    setAudioOptions
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_fmtech_fmlive_jni_PushNative_setAudioOptions
  (JNIEnv *env, jobject jobj, jint sampleRateInHz, jint numChannels){
	audio_encode_handle = faacEncOpen(sampleRateInHz, numChannels,&inputSamples,&maxOutputBytes);
	if(!audio_encode_handle){
		LOGE("%s","-------open audio_encode failed");
		return;
	}

	//������Ƶ�������
	faacEncConfigurationPtr p_config = faacEncGetCurrentConfiguration(audio_encode_handle);
	p_config->mpegVersion = MPEG4;
	p_config->allowMidside = 1;
	p_config->aacObjectType = LOW;
	p_config->outputFormat = 0; //����Ƿ����ADTSͷ
	p_config->useTns = 1; //ʱ����������,��ž���������
	p_config->useLfe = 0;
//	p_config->inputFormat = FAAC_INPUT_16BIT;
	p_config->quantqual = 100;
	p_config->bandWidth = 0; //Ƶ��
	p_config->shortctl = SHORTCTL_NORMAL;

	if(!faacEncSetConfiguration(audio_encode_handle, p_config)){
		LOGE("%s","set faac encode configure failed");
		return;
	}
	LOGI("%s","set faac encode configure success");
}
