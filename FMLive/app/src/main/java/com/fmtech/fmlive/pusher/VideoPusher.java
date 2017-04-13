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
 * @create_date 2017年3月18日 下午9:30:42
 *
 *==================================================================
 */
package com.fmtech.fmlive.pusher;

import java.util.List;

import com.fmtech.fmlive.jni.PushNative;
import com.fmtech.fmlive.params.VideoParams;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.view.SurfaceHolder;

public class VideoPusher extends Pusher implements SurfaceHolder.Callback,PreviewCallback{

	private Camera mCamera;
	private VideoParams mVideoParams;
	private SurfaceHolder mSurfaceHolder;
	private byte[] mCallbackBuffer;
	private boolean isPushing = false;
	private PushNative mPushNative;
	
	public VideoPusher(SurfaceHolder surfaceHolder, VideoParams videoParams, PushNative pushNative) {
		mVideoParams = videoParams;
		mPushNative = pushNative;
		mSurfaceHolder = surfaceHolder;
		mSurfaceHolder.addCallback(this);
	}

	@Override
	public void startPush() {
		mPushNative.setVideoOptions(mVideoParams.getWidth(), mVideoParams.getHeight(), mVideoParams.getBitrate(), mVideoParams.getFps());
		isPushing = true;
	}

	@Override
	public void stopPush() {
		isPushing = false;
		
	}

	@Override
	public void release() {
		stopPreview();
	}
	
	public void switchCamera(){
		if(mVideoParams.getCameraId() == CameraInfo.CAMERA_FACING_BACK){
			mVideoParams.setCameraId(CameraInfo.CAMERA_FACING_FRONT);
		}else{
			mVideoParams.setCameraId(CameraInfo.CAMERA_FACING_BACK);
		}
		stopPreview();
		startPreview();
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		startPreview();
	}

	@SuppressWarnings("deprecation")
	private void startPreview() {
		try {
			mCamera = Camera.open(mVideoParams.getCameraId());
			mCamera.setPreviewDisplay(mSurfaceHolder);
			mCamera.setPreviewCallbackWithBuffer(this);
			Camera.Parameters parameters = mCamera.getParameters();
//			parameters.setPreviewFormat(ImageFormat.NV21);//YUV
			List<Integer> sIntegers = parameters.getSupportedPreviewFormats();
			parameters.setPreviewSize(mVideoParams.getWidth(), mVideoParams.getHeight());
//			List<int[]> list =parameters.getSupportedPreviewFpsRange();
//			parameters.setPreviewFpsRange(24, mVideoParams.getFps());
//			List<Size> sizes = parameters.getSupportedPreviewSizes();
//			for(Size size:sizes){
//				System.out.println("-------previewsize#width:"+size.width+", height:"+size.height);
//			}
			for(Integer size:sIntegers){
			System.out.print("-------"+size);
			}			
			mCamera.setParameters(parameters);
			
			//获取预览图像数据
			mCamera.setDisplayOrientation(90);
			mCallbackBuffer = new byte[mVideoParams.getWidth()*mVideoParams.getHeight()*4];
			mCamera.addCallbackBuffer(mCallbackBuffer);
			
			mCamera.startPreview();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private void stopPreview() {
		if(null != mCamera){
			mCamera.stopPreview();
			mCamera.release();
			mCamera = null;
		}
	}
	
	
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		//回调方法中获取图像数据，调用native方法进行处理
		if(null != mCamera){
			mCamera.addCallbackBuffer(mCallbackBuffer);
		}
		
		if(isPushing && null != mPushNative){
			mPushNative.fireVideo(data);
//			System.out.println("-------视频编码:"+System.currentTimeMillis());
		}
	}
	
}
