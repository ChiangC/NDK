package com.fmtech.ffmpeg;

/**
 * @author Chiang.CMBA
 * @date Created on: 2017-3-4 下午3:28:46
 * @version v1.0
 * @Description: TODO(What's the class used for?) 
 */

public class VideoUtils {

	public native static void decode(String input, String output);

	static{
		System.loadLibrary("avutil-54");
		System.loadLibrary("swresample-1");
		System.loadLibrary("avcodec-56");
		System.loadLibrary("avformat-56");
		System.loadLibrary("swscale-3");
		System.loadLibrary("postproc-53");
		System.loadLibrary("avfilter-5");
		System.loadLibrary("avdevice-56");
		System.loadLibrary("fmtech_ffmpeg");
	}
	
}
