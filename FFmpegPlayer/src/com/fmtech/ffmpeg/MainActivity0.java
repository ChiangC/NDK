package com.fmtech.ffmpeg;

import java.io.File;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;




/**
 * @author Chiang.CMBA
 * @date Created on: 2017-3-4 下午3:18:56
 * @version v1.0
 * @Description: TODO(What's the class used for?) 
 */

public class MainActivity0 extends Activity{

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.activity_main);
		
	}
	
	public void mDecode(View btn){
		String input = new File(Environment.getExternalStorageDirectory(),"input.mp4").getAbsolutePath();
		String output = new File(Environment.getExternalStorageDirectory(),"CHIANG" + File.separator + "output_1280x720_yuv420p.yuv").getAbsolutePath();
		VideoUtils.decode(input, output);
	}
	
}
