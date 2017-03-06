package com.fmtech.ffmpeg;

import java.io.File;

import com.fmtech.ffmpeg.view.VideoView;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.view.Surface;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

public class MainActivity extends Activity {

	private FMPlayer player;
	private VideoView videoView;
	private Spinner sp_video;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		videoView = (VideoView) findViewById(R.id.video_view);
		sp_video = (Spinner) findViewById(R.id.sp_video);
		player = new FMPlayer();
		//多种格式的视频列表
		String[] videoArray = getResources().getStringArray(R.array.video_list);
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, 
				android.R.layout.simple_list_item_1, 
				android.R.id.text1, videoArray);
		sp_video.setAdapter(adapter);
	}

	public void mPlay(View btn){
		String video = sp_video.getSelectedItem().toString();
		String input = new File(Environment.getExternalStorageDirectory(),video).getAbsolutePath();
		//Surface传入到Native函数中，用于绘制
		Surface surface = videoView.getHolder().getSurface();
		player.render(input, surface);
	}
}
