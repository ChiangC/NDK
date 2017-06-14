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
 * @create_date 2017年3月18日 下午11:08:22
 *
 *==================================================================
 */
package com.fmtech.fmlive.params;

public class AudioParams {

	private int sampleRateInHz = 44100;

	private int channel = 1;

	/**
	 * @return the sampleRateInHz
	 */
	public int getSampleRateInHz() {
		return sampleRateInHz;
	}

	/**
	 * @param sampleRateInHz the sampleRateInHz to set
	 */
	public void setSampleRateInHz(int sampleRateInHz) {
		this.sampleRateInHz = sampleRateInHz;
	}

	/**
	 * @return the channel
	 */
	public int getChannel() {
		return channel;
	}

	public void setChannel(int channel) {
		this.channel = channel;
	}

	public AudioParams() {

	}

	public AudioParams(int sampleRateInHz, int channel) {
		super();
		this.sampleRateInHz = sampleRateInHz;
		this.channel = channel;
	}


}
