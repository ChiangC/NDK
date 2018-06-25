package com.fmtech.giflib;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private GifHandler mGifHandler;
    private Bitmap mBitmap;
    private ImageView mImageView;
    private Handler mHandler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            int nextFrame = mGifHandler.updateFrame(mBitmap);
            mImageView.setImageBitmap(mBitmap);
            mHandler.sendEmptyMessageDelayed(1, nextFrame);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mImageView= (ImageView) findViewById(R.id.imageview);
    }

    public void ndkLoadGif(View view) {
        if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE}, 100);
        } else {
            loadGif();
        }
    }

    private void loadGif(){
        File gifFile = new File(Environment.getExternalStorageDirectory(), "ai.gif");
        mGifHandler = new GifHandler(gifFile.getAbsolutePath());

        int width = mGifHandler.getWidth();
        int height = mGifHandler.getHeight();
        mBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        int nextFrame = mGifHandler.updateFrame(mBitmap);

        mHandler.sendEmptyMessageDelayed(1, nextFrame);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if(grantResults[0] == PackageManager.PERMISSION_GRANTED){
            loadGif();
        }
    }
}
