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
 * @create_date 2017年3月18日 下午10:15:38
 *
 *==================================================================
 */


package com.fmtech.fmlive.params;

public class VideoParams {
	
	private int width;
	private int height;
	private int bitrate = 480000;//码率480kbps
	private int fps = 25;//帧频默认25帧/s
	private int cameraId;
	
	/**
	 * @param width
	 * @param height
	 * @param cameraId
	 */
	public VideoParams(int width, int height, int cameraId) {
		super();
		this.width = width;
		this.height = height;
		this.cameraId = cameraId;
	}

	public int getWidth() {
		return width;
	}
	
	public void setWidth(int width) {
		this.width = width;
	}
	
	public int getHeight() {
		return height;
	}
	
	public void setHeight(int height) {
		this.height = height;
	}

	public int getBitrate() {
		return bitrate;
	}

	public void setBitrate(int bitrate) {
		this.bitrate = bitrate;
	}

	public int getFps() {
		return fps;
	}

	public void setFps(int fps) {
		this.fps = fps;
	}

	public int getCameraId() {
		return cameraId;
	}
	
	public void setCameraId(int cameraId) {
		this.cameraId = cameraId;
	}
	
	
}
