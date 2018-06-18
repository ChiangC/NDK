package com.fmtech.incrementalupdate;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private static String TAG = "BsPatch";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        checkPermission();

    }
    private void checkPermission() {
        if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE}, 100);
        } else {
            init();
        }
    }

    private void init(){
        if (ApkUtils.getVersionCode(MainActivity.this) < 2.0) {
            Log.d(TAG,"不是最新的版本号 开始更新 ");
            new ApkUpdateTask().execute();
        } else {
            Log.d(TAG ," 最新版本号 无需更新");
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if(grantResults[0] == PackageManager.PERMISSION_GRANTED){
            init();
        }
    }

   class ApkUpdateTask extends AsyncTask<Void, Void, Boolean>{
       @Override
       protected Boolean doInBackground(Void... voids) {
           int result = -1;
           File pathFile = DownloadUtils.download(Constants.URL_PATCH_DOWNLOAD, MainActivity.this);
           if(null != pathFile){
               String oldApkPath = ApkUtils.getSourceApkPath(MainActivity.this);
               String newApkPath = Constants.NEW_APK_PATH;
               String patchPath = pathFile.getAbsolutePath();

               result = BsPatch.patch(oldApkPath, newApkPath, patchPath);
           }

           return result == 0;
       }

       @Override
       protected void onPostExecute(Boolean aBoolean) {
           super.onPostExecute(aBoolean);
           if(aBoolean){
               Log.d(TAG," Installing new apk. ");
               ApkUtils.installApk(MainActivity.this, Constants.NEW_APK_PATH);
           }
       }
   }

}
