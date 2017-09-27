#include "XMediaEncode.h"
extern "C"
{
#include <libswscalse/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")

#include <iostream>
using namespace std

#if defined WIN32 || defined _WIN32
#include <windows.h>
#endif


static int XGetCpuNum()
{
#if defined WIN32 || defined _WIN32
    SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);

        return (int)sysinfo.dwNumberOfProcessors;
}
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

    if(numCPU < 1)
    {
        mib[1] = HW_NCPU;
        sysctl(mib, 2, &numCPU, &len, NULL, 0);

        if(numCPU)
            numCPU = 1;
    }

    return (int)numCPU;
#else
    return


class CXMediaEncode:pubic XMediaEncode
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
   }

   bool InitScale()
   {
        vsc = sws_getCachedContext(vsc,
			srcW, srcH, AV_PIX_FMT_BGR24,//源宽、高、像素格式
			dstW, dstH, AV_PIX_FMT_YUV420P,//目标宽、高、像素格式
			SWS_BICUBIC,//尺寸变化使用算法
			0, 0, 0);
		if(!vsc)
		{
		    cout<<"sws_getCachedContext failed!"<<endl;
		    return false;
		}

		yuv = av_frame_alloc();
        yuv->format = AV_PIX_FMT_YUV420P;
        yuv->width = srcW;
        yuv->height = srcH;
        yuv->pts = 0;

        //分配yuv空间
        int ret = av_frame_get_buffer(yuv, 32);
        if (ret != 0){
            return false;
        }

        return true;
   }


    AVFrame* RGBToYUV(char *rgb)
    {
        //rgb to yuv
        //输入的数据结构
        uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
        //bgrbgrbgr
        //plane indata[0]:bbbbb indata[1]:ggggg indata[2]:rrrrr
        indata[0] = (uint8_t *)rgb;
        int insize[AV_NUM_DATA_POINTERS] = { 0 };
        //一行(宽)数据的字节数
        insize[0] = inWidth * inPixSize;

        int h = sws_scale(vsc, indata, insize, 0, inHeight, //源数据
            yuv->data, yuv->linesize);

        if (h <= 0){
            return NULL;
        }
        return yuv;
    }

    bool InitVideoCodec()
    {

    }

private:
    SwsContext *vsc = NULL;//像素格式转换上下文
    AVFrame *yuv = NULL;
    //编码器上下文

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