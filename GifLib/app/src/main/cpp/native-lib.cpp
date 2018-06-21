#include <jni.h>
#include <string>
#include <malloc.h>
#include <string.h>
#include "gif_lib.h"
//#include <android/log.h>
#include <android/bitmap.h>

#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)

typedef struct GifBean{
    int current_frame;
    int total_frame;
    int *delays;
}GifBean;

extern "C" {

void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo bitmapInfo, void *pixels){

    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];

    GifImageDesc frameInfo = savedImage.ImageDesc;
    int* px = (int*)pixels;
    //
    int* line;

    int pointPixel;
    GifByteType gifByteType;
    GifColorType gifColorType;
    ColorMapObject* colorMapObject = frameInfo.ColorMap;
    px = (int*)((char*)px + bitmapInfo.stride * frameInfo.Top);
    for(int y = frameInfo.Top; y < frameInfo.Top + frameInfo.Height; y++){
        line = px;
        for(int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; x++){
            pointPixel = (y - frameInfo.Top) * frameInfo.Width + (x - frameInfo.Left);
            gifByteType = savedImage.RasterBits[pointPixel];
            gifColorType = colorMapObject->Colors[gifByteType];
            line[x] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);
        }
        px = (int*)((char*)px + bitmapInfo.stride);
    }

}

JNIEXPORT jlong JNICALL
Java_com_fmtech_giflib_GifHandler_loadGif(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    int error;
    /** Open a new GIF file for read, given by its name.Returns dynamically allocated GifFileType pointer which serves as the GIF
info record.*/
    GifFileType* gifFileType = DGifOpenFileName(path, &error);
    //init gifFileType;
    DGifSlurp(gifFileType);
    GifBean *gifBean = (GifBean*) malloc(sizeof(GifBean));

    memset(gifBean, 0, sizeof(GifBean));
    //
    gifFileType->UserData = gifBean;

    gifBean->delays = (int*)malloc(sizeof(int) * gifFileType->ImageCount);
    memset(gifBean->delays, 0, sizeof(int) * gifFileType->ImageCount);
    gifBean->total_frame = gifFileType->ImageCount;

    ExtensionBlock* ext;
    for(int i = 0; i<gifFileType->ImageCount; ++i){
        SavedImage frame = gifFileType->SavedImages[i];
        for(int j = 0; j < frame.ExtensionBlockCount; ++j){
            //
            if(frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE){
                ext = &frame.ExtensionBlocks[j];
                break;
            }
        }
        if(ext){
            //
            int frame_delay = 10 * (ext->Bytes[2]<< 8 | ext->Bytes[1]);
            gifBean->delays[i] = frame_delay;
        }
    }

    env->ReleaseStringUTFChars(path_, path);
    return (jlong)gifFileType;
}

JNIEXPORT jint JNICALL
Java_com_fmtech_giflib_GifHandler_getWidth(JNIEnv *env, jobject instance, jlong gifAddr) {
    GifFileType* gifFileType = (GifFileType*)gifAddr;
    return gifFileType->SWidth;
}

JNIEXPORT jint JNICALL
Java_com_fmtech_giflib_GifHandler_getHeight(JNIEnv *env, jobject instance, jlong gifAddr) {
    GifFileType* gifFileType = (GifFileType*) gifAddr;
    return gifFileType->SHeight;
}


JNIEXPORT jint JNICALL
Java_com_fmtech_giflib_GifHandler_updateFrame(JNIEnv *env, jobject instance, jobject bitmap,
                                              jlong gifAddr) {
    GifFileType *gifFileType = (GifFileType*)gifAddr;
    GifBean *gifBean = (GifBean*)gifFileType->UserData;

    //
    AndroidBitmapInfo bitmapInfo;
    void *pixels;
    AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
    //Given a java bitmap object, attempt to lock the pixel address.
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    drawFrame(gifFileType, gifBean, bitmapInfo, pixels);

    //
    gifBean->current_frame += 1;

    if(gifBean->current_frame >= gifBean->total_frame - 1){
        gifBean->current_frame = 0;
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    return gifBean->delays[gifBean->current_frame];
}

}
