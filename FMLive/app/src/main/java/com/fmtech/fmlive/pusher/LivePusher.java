package com.fmtech.fmlive.pusher;

import com.fmtech.fmlive.jni.PushNative;
import com.fmtech.fmlive.listener.LiveStateChangeListener;
import com.fmtech.fmlive.params.AudioParams;
import com.fmtech.fmlive.params.VideoParams;

import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;

public class LivePusher implements Callback {
	private SurfaceHolder mSurfaceHolder;
	private VideoParams mVideoParams;
	private VideoPusher mVideoPusher;
	private AudioPusher mAudioPusher;
	private PushNative mPushNative;

	public LivePusher(SurfaceHolder surfaceHolder){
		mSurfaceHolder = surfaceHolder;
		mSurfaceHolder.addCallback(this);
		mPushNative = new PushNative();
		prepare();
	}

	private void prepare(){
		VideoParams videoParams = new VideoParams(1280,720, CameraInfo.CAMERA_FACING_BACK);
		mVideoPusher = new VideoPusher(mSurfaceHolder, videoParams, mPushNative);

		AudioParams audioParams = new AudioParams();
		mAudioPusher = new AudioPusher(audioParams, mPushNative);

	}

	public void switchCamera(){
		mVideoPusher.switchCamera();
	}

	public void startPush(String url, LiveStateChangeListener listener) {
		mVideoPusher.startPush();
		mAudioPusher.startPush();
		mPushNative.startPush(url);
		mPushNative.setLiveStateChangeListener(listener);
	}

	public void stopPush() {
		mVideoPusher.stopPush();
		mAudioPusher.stopPush();
		mPushNative.stopPush();
	}

	private void release(){
		mVideoPusher.release();
		mAudioPusher.release();
		mPushNative.release();
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {

	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		stopPush();
		release();
	}



}
