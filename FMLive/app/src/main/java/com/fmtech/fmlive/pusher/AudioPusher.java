package com.fmtech.fmlive.pusher;

import com.fmtech.fmlive.jni.PushNative;
import com.fmtech.fmlive.params.AudioParams;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;

public class AudioPusher extends Pusher{

	private AudioParams mAudioParams;
	private AudioRecord mAudioRecord;
	private boolean isPushing;
	private int minBufferSize;
	private PushNative mPushNative;
	
	public AudioPusher(AudioParams audioParams, PushNative pushNative) {
		mAudioParams = audioParams;
		mPushNative = pushNative;
		int channelConfig = audioParams.getChannel() == 1?
				AudioFormat.CHANNEL_IN_MONO:AudioFormat.CHANNEL_IN_STEREO;
		minBufferSize = AudioRecord.getMinBufferSize(mAudioParams.getSampleRateInHz(), channelConfig, AudioFormat.ENCODING_PCM_16BIT);
		mAudioRecord = new AudioRecord(AudioSource.MIC, mAudioParams.getSampleRateInHz(), 
				channelConfig, AudioFormat.ENCODING_PCM_16BIT, minBufferSize);
				
	}
	
	@Override
	public void startPush() {
		mPushNative.setAudioOptions(mAudioParams.getSampleRateInHz(), mAudioParams.getChannel());
		isPushing = true;
		new Thread(new AudioRecordTask()).start();
		
	}

	@Override
	public void stopPush() {
		isPushing = false;
		mAudioRecord.stop();
	}
	
	@Override
	public void release() {
		if(null != mAudioRecord){
			mAudioRecord.release();
			mAudioRecord = null;
		}
	}
	
	class AudioRecordTask implements Runnable{

		@Override
		public void run() {
			mAudioRecord.startRecording();
			
			while(isPushing){
				byte[] audioData = new byte[minBufferSize];
				int length = mAudioRecord.read(audioData, 0, audioData.length);
				if(length > 0){
					mPushNative.fireAudio(audioData, length);
//					System.out.println("-------ÒôÆµ±àÂë");
				}
			}
		}
		
	}

}
