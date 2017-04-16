/**
* Copyright (c) 2015-2016 VisionStar Information Technology (Shanghai) Co., Ltd. All Rights Reserved.
* EasyAR is the registered trademark or trademark of VisionStar Information Technology (Shanghai) Co., Ltd in China
* and other countries for the augmented reality technology developed by VisionStar Information Technology (Shanghai) Co., Ltd.
*/

package cn.easyar.samples.helloar;

import android.content.res.Configuration;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Toast;

import cn.easyar.engine.EasyAR;


public class MainActivity extends AppCompatActivity{

    /*
    * Steps to create the key for this sample:
    *  1. login www.easyar.com
    *  2. create app with
    *      Name: HelloAR
    *      Package Name: cn.easyar.samples.helloar
    *  3. find the created item in the list and show key
    *  4. set key string bellow
    */
    static String key = "PXTHB1AceAu4JZVoGs2F7MsIJEeKPsOGX6MSCZJ0yJ7KVNlS7jk5FtkvTUx7HNQ75FYidT60UWhwOqi9J009Q5AeGkbLyMx3PJSlffa5fec74e972ec7e79c62623370d194zD1fyWc2aPmGILsvZCouT7FK8s5evoE7zL5JRNzpWZfPcGL3H0bmqzQTzQveQiQXuZN9";

    static {
        System.loadLibrary("EasyAR");
        System.loadLibrary("HelloARNative");
    }

    public static native void nativeInitGL();
    public static native void nativeResizeGL(int w, int h);
    public static native void nativeRender();
    private native boolean nativeInit(ARCallback callback);
    private native void nativeDestory();
    private native void nativeRotationChange(boolean portrait);

    private ARCallback mARCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mARCallback = new ARCallback() {
            @Override
            public void onTrackSuccess(int size) {
                System.out.println("-------size:"+size);
            }

            @Override
            public void onInitSuccess(int size) {
                Toast.makeText(MainActivity.this, "AR Init Successed.",Toast.LENGTH_SHORT).show();
            }
        };

        EasyAR.initialize(this, key);
        nativeInit(mARCallback);

        GLView glView = new GLView(this);
        glView.setRenderer(new Renderer());
        glView.setZOrderMediaOverlay(true);

        ((ViewGroup) findViewById(R.id.preview)).addView(glView, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        nativeRotationChange(getWindowManager().getDefaultDisplay().getRotation() == android.view.Surface.ROTATION_0);
    }

    @Override
    public void onConfigurationChanged(Configuration config) {
        super.onConfigurationChanged(config);
        nativeRotationChange(getWindowManager().getDefaultDisplay().getRotation() == android.view.Surface.ROTATION_0);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        nativeDestory();
    }
    @Override
    protected void onResume() {
        super.onResume();
        EasyAR.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        EasyAR.onPause();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();

        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
