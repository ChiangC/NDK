extern "C"
{
#include "libavformat/avformat.h"//头文件只是在编译时，用来语法检测，能找到函数的声明
#include "libavutil/time.h"
}

#include <iostream>
using namespace std;

//添加库文件，已找到函数的定义
#pragma comment(lib,"avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

int XError(int errNum){
	char buf[1024] = { 0 };
	av_strerror(errNum, buf, sizeof(buf));
	cout << buf << endl;
	return -1;
}

static double r2d(AVRational r){
    return r.num == 0 || r.den == 0?0. : (double)r.num/(double)r.den;
}

int main(int argc, char *argv[])
{
	const char *inUrl = "FMLive.mp4";
	const char *outUrl = "rtmp://106.14.33.215:1935/live/fmlive";


	//初始化所有封装解封装 flv mp4 mov mp3
	av_register_all();

	//初始化网络库
	avformat_network_init();

	//1.打开文件解封装
	//输入封装上下文
	AVFormatContext *ifmt_ctx = NULL;
	
	//打开文件，解封文件头
	//传的是ifmt_ctx的地址，说明ifmt_ctx由我们进行分配，这样，就要注意进行释放。
	int ret = avformat_open_input(&ifmt_ctx,inUrl, NULL, NULL);
	if (ret < 0){
		return XError(ret);
	}
	cout << "Open file '" << inUrl << "' success." << endl;

	//获取音频视频流信息, h264 flv
	ret = avformat_find_stream_info(ifmt_ctx, NULL);
	if (ret < 0){
		return XError(ret);
	}

	av_dump_format(ifmt_ctx, 0, inUrl, 0);

	///////////////////////////////////////////////////
	//输出流

	//创建输出流上下文
	AVFormatContext *ofmt_ctx = NULL;
	ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, "mp4", outUrl);
	if (!ofmt_ctx){
		return XError(ret);
	}
	cout << "ofmt_ctx create success!" << endl;
	
	//配置输出流
	//遍历输入的AVStream
	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++){
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, ifmt_ctx->streams[i]->codec->codec);
		if (!out_stream){
			return XError(0);
		}

		//复制配置信息,用于mp4
		ret = avcodec_copy_context(out_stream->codec, ifmt_ctx->streams[i]->codec);
		
		//复制配置信息,新版本
		//ret = avcodec_parameters_copy(out_stream->codecpar, ifmt_ctx->streams[i]->codecpar);
		out_stream->codec->codec_tag = 0;
	

	}

	av_dump_format(ofmt_ctx, 0, outUrl, 1);

	///////////////////////////////////////////////////
	//rtmp推流
	
	
	//打开IO
    ret = avio_open(&ofmt_ctx->pb, outUrl, AVIO_FLAG_WRITE);
    if(!ofmt_ctx->pb){
        return XError(ret);
    }
	
	//写入头信息
ret = avformat_write_header(ofmt_ctx, NULL);
    if(ret < 0){
        return XError(ret);
    }
	
	cout<< "avformat_write_header"<< endl;

    //推流每一帧数据
    AVPacket pkt;
    long long startTime = av_gettime();//获取微妙时间戳

    while(true){
        ret = av_read_frame(ifmt_ctx, &pkt);
        if(ret != 0){
            break;
        }

        cout<< pkt.pts << " "<<flush;
        //计算转换pts dts
        AVRational itime = ifmt_ctx->streams[pkt.stream_index]->time_base;
        AVRational otime = ofmt_ctx->streams[pkt.stream_index]->time_base;

        pkt.pts = av_rescale_q_rnd(pkt.pts, itime, otime, (AVRational)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, itime, otime, (AVRational)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescalse_q_rnd(pkt.duration, itime, otime, (AVRational)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.position = -1;

        //视频帧推送速度
        if(ifmt_ctx->streams[pkt.stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            AVRational tb = ifmt_ctx->streams[pkt.stream_index]->time_base;
            //已经过去的时间
            long long now = av_gettime() - startTime;
            long long dts = 0;
            dts = pkt.dts * (1000*1000*r2d(tb));
            if(dts > now){
                av_usleep(dts - now);
            }

        }
        if(ret < 0){
            return XError(ret);
        }

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);


        av_packet_unref(&pkt);
    }
	

	getchar();
	return 0;
}
