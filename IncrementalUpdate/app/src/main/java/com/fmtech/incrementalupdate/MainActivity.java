package com.fmtech.incrementalupdate;

import android.os.AsyncTask;
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
        if (ApkUtils.getVersionCode(MainActivity.this) < 2.0) {
            Log.d(TAG,"不是最新的版本号 开始更新 ");
            new ApkUpdateTask().execute();
        } else {
            Log.d(TAG ," 最新版本号 无需更新");
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
               System.out.println("Installing new apk.");
               ApkUtils.installApk(MainActivity.this, Constants.NEW_APK_PATH);
           }
       }
   }

}
