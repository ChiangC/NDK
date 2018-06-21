package com.fmtech.giflib;

import android.graphics.Bitmap;

/**
 * ==================================================================
 * Copyright (C) 2018 FMTech All Rights Reserved.
 *
 * @author Drew.Chiang
 * @version v1.0.0
 * @email chiangchuna@gmail.com
 * @create_date 2018/6/21 19:28
 * <p>
 * ==================================================================
 */

public class GifHandler {
    static {
        System.loadLibrary("native-lib");
    }

    private long gifAddr;

    public GifHandler(String path){
        gifAddr = loadGif(path);
    }

    public int getWidth(){
        return getWidth(gifAddr);
    }

    public int getHeight(){
        return getHeight(gifAddr);
    }

    public int updateFrame(Bitmap bitmap){
        return updateFrame(bitmap, gifAddr);
    }

    private native long loadGif(String path);

    public native int getWidth(long gifAddr);

    public native int getHeight(long gifAddr);

    public native int updateFrame(Bitmap bitmap, long gifAddr);

}
