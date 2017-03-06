package com.fmtech.ffmpeg.view;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;


/**
 * @author Chiang.CMBA
 * @date Created on: 2017-3-5 上午11:49:59
 * @version v1.0
 * @Description: TODO(What's the class used for?) 
 */

public class VideoView extends SurfaceView {

	public VideoView(Context context){
		this(context, null);
	}
	
	public VideoView(Context context, AttributeSet attrs) {
		super(context, attrs);
		init();
	}

	private void init(){
		SurfaceHolder surfaceHolder = getHolder();
		surfaceHolder.setFormat(PixelFormat.RGB_888);
	}
	
	
}
