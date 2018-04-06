/**
 *==================================================================
 * Copyright (C) 2017 FMTech All Rights Reserved.
 *
 * @author Drew.Chiang
 *
 * @email chiangchuna@gmail.com
 *
 * @version v1.0.0
 *
 * @create_date 2017年3月19日 上午9:37:27
 *
 *==================================================================
 */

package com.fmtech.fmlive.jni;

import com.fmtech.fmlive.listener.LiveStateChangeListener;

public class PushNative {
	public static final int CONNECT_FAILED = 101;
	public static final int INIT_FAILED = 102;

	LiveStateChangeListener mLiveStateChangeListener;

	/*static{
		System.loadLibrary("FMLive");
	}*/
	public native void fireVideo(byte[] data);

	public native void fireAudio(byte[] data, int len);

	public native void startPush(String url);

	public native void stopPush();

	public native void release();

	public native void setVideoOptions(int width, int height, int bitrate, int fps);

	public native void setAudioOptions(int sampleRateInHz, int channel);

	public void setLiveStateChangeListener(LiveStateChangeListener listener){
		mLiveStateChangeListener = listener;
	}

	public void removeLiveStateChangeListener(){
		mLiveStateChangeListener = null;
	}

	public void throwNativeError(int code){
		if(null != mLiveStateChangeListener){
			mLiveStateChangeListener.onError(code);
		}
	}
}
