extern "C"
{
#include "libavformat/avformat.h"//ͷ�ļ�ֻ���ڱ���ʱ�������﷨��⣬���ҵ�����������
}

#include <iostream>
using namespace std;

//��ӿ��ļ������ҵ������Ķ���
#pragma comment(lib,"avformat.lib")
#pragma comment(lib, "avutil.lib")

int XError(int errNum){
	char buf[1024] = { 0 };
	av_strerror(errNum, buf, sizeof(buf));
	cout << buf << endl;
	return -1;
}

int main(int argc, char *argv[])
{
	const char *inUrl = "FMLive.mp4";
	const char *outUrl = "rtmp://106.14.33.215:1935/live/fmlive";


	//��ʼ�����з�װ���װ flv mp4 mov mp3
	av_register_all();

	//��ʼ�������
	avformat_network_init();

	//1.���ļ����װ
	//�����װ������
	AVFormatContext *ifmt_ctx = NULL;
	
	//���ļ�������ļ�ͷ
	//������ifmt_ctx�ĵ�ַ��˵��ifmt_ctx�����ǽ��з��䣬��������Ҫע������ͷš�
	int ret = avformat_open_input(&ifmt_ctx,inUrl, NULL, NULL);
	if (ret < 0){
		return XError(ret);
	}
	cout << "Open file '" << inUrl << "' success." << endl;

	//��ȡ��Ƶ��Ƶ����Ϣ, h264 flv
	ret = avformat_find_stream_info(ifmt_ctx, NULL);
	if (ret < 0){
		return XError(ret);
	}

	av_dump_format(ifmt_ctx, 0, inUrl, 0);

	///////////////////////////////////////////////////
	//�����

	//���������������
	AVFormatContext *ofmt_ctx = NULL;
	ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, "mp4", outUrl);
	if (!ofmt_ctx){
		return XError(ret);
	}
	cout << "ofmt_ctx create success!" << endl;
	
	//���������
	//���������AVStream
	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++){
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, ifmt_ctx->streams[i]->codec->codec);
		if (!out_stream){
			return XError(0);
		}

		//����������Ϣ,����mp4
		ret = avcodec_copy_context(out_stream->codec, ifmt_ctx->streams[i]->codec);
		
		//����������Ϣ,�°汾
		//ret = avcodec_parameters_copy(out_stream->codecpar, ifmt_ctx->streams[i]->codecpar);
		out_stream->codec->codec_tag = 0;
	

	}

	av_dump_format(ofmt_ctx, 0, outUrl, 1);

	///////////////////////////////////////////////////
	//rtmp����
	//д��ͷ��Ϣ


	getchar();
	return 0;
}