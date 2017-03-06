package com.fmtech.ffmpeg;

import android.view.Surface;

/**
 * @author Chiang.CMBA
 * @date Created on: 2017-3-5 上午11:51:38
 * @version v1.0
 * @Description: TODO(What's the class used for?) 
 */

public class FMPlayer {

	public native void render(String input, Surface surface);

	static{
		System.loadLibrary("avutil-54");
		System.loadLibrary("swresample-1");
		System.loadLibrary("avcodec-56");
		System.loadLibrary("avformat-56");
		System.loadLibrary("swscale-3");
		System.loadLibrary("postproc-53");
		System.loadLibrary("avfilter-5");
		System.loadLibrary("avdevice-56");
		System.loadLibrary("yuv");
		System.loadLibrary("fmtech_ffmpeg");
	}
}
