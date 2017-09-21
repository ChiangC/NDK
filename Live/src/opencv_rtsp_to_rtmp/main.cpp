#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <iostream>
extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}
//预处理失败，找不到头文件；编译失败语法错误；链接失败，找不到lib文件；执行失败，找不到动态链接库。
using namespace cv;
using namespace std;

#pragma comment(lib, "opencv_world320d.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")

int main(int argc, char *argv)
{
    const char *inUrl = "rtsp://admin:ASZSCS@192.168.3.3";
    const char *outUrl = "rtmp://106.14.33.215:1935/live/fmlive";

    Mat frame;
    namedWindow("video");

    //像素格式转换上下文
    SwsContext *vsc = NULL;

    //输出的数据结构
    AVFrame *yuv = NULL;

    //编码器上下文
    AVCodecContext *codec_ctx = NULL;

	VideoCapture cam;

    try{
    ////////////////////////////////////////////////////////////
        ///1.使用opencv打开rtsp相机
    
        cam.open(inUrl);
        if(!cam.isOpened()){
            throw exception("Cam open failed!");
        }
        cout <<inUrl<< " cam open success!" << endl;


        int srcW = cam.get(CAP_PROP_FRAME_WIDTH);
        int srcH = cam.get(CAP_PROP_FRAME_HEIGHT);
        int dstW = srcW, dstH = srcH;
        int fps = cam.get(CAP_PROP_FPS);

        ///2.初始化格式转换上下文
        vsc = sws_getCachedContext(vsc,
                                   srcW, srcH, AV_PIX_FMT_BGR24,//源宽、高、像素格式
                                   dstW, dstH, AV_PIX_FMT_YUV420P,//目标宽、高、像素格式
                                   SWS_BICUBIC,//尺寸变化使用算法
                                   0,0,0);


        if(!vsc){
            throw exception("sws_getCachedContext failed.");
        }

        ///3.初始化输出的数据结构
        yuv = av_frame_alloc();
        yuv->format = AV_PIX_FMT_YUV420P;
        yuv->width = srcW;
        yuv->height = srcH;
        yuv->pts = 0;

        //分配yuv空间
        int ret = av_frame_get_buffer(yuv, 32);
        if(ret  != 0){
            char buf[1024] = {0};
            av_strerror(ret, buf, sizeof(buf) - 1);
            throw exception(buf);
        }

        ///4.初始化编码上下文
        //        avcodec_register_all();
        //1.找到编码器
        AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);

        if(!codec){
            throw exception("Can't find h264 encoder!");
        }

        //2.创建编码器上下文
        codec_ctx = avcodec_alloc_context3(codec);
        if(!codec_ctx){
            throw exception("avcodec_alloc_context3 failed!");
        }

        //3.配置编码器参数
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;//全局参数
        codec_ctx->codec_id = codec->id;
        codec_ctx->thread_count = 8;//编码线程数量
        codec_ctx->bit_rate = 200*1024*8;//压缩后每秒视频的bit位大小,通过压缩率控制视频的码率；200KB
        codec_ctx->width = srcW;
        codec_ctx->height = srcH;
        codec_ctx->time_base = {1,fps};//pts以什么数进行计算
        codec_ctx->framerate = {fps,1};//帧率
        codec_ctx->gop_size = 50;//画面组的大小，多少帧一个关键帧

        //4.打开编码器上下文





        for(;;){
            //读取rtsp视频帧，解码视频帧
            if(!cam.grab()){//只做解码
                continue;
            }

            //yuv转换为rgb
            if(!cam.retrieve(frame)){
                continue;
            }

            imshow("video", frame);
            waitKey(1);


            //rgb to yuv

            //输入的数据结构
            uint8_t *indata[AV_NUM_DATA_POINTERS] = {0};
            //bgrbgrbgr
            //plane indata[0]:bbbbb indata[1]:ggggg indata[2]:rrrrr
            indata[0] = frame.data;
            int insize[AV_NUM_DATA_POINTERS] = {0};
            //一行(宽)数据的字节数
            insize[0] = frame.cols * frame.elemSize();

            int h = sws_scale(vsc, indata, insize, 0, frame.rows, //源数据
                              yuv->data, yuv->linesize);

            if(h <= 0){
                continue;
            }
            cout << h << " " << flush;



        }


    }catch(exception &ex){
        if(cam.isOpened()){
            cam.release();
        }

        if(vsc){
            sws_freeContext(vsc);
            vsc = NULL;
        }

        if(codec_ctx){
            avcodec_free_context(&codec_ctx);
        }


        cerr << ex.what() << endl;
    }




    getchar();
    return 0;
}