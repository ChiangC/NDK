package com.fmtech.incrementalupdate;

import android.content.Context;
import android.os.Environment;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * ==================================================================
 * Copyright (C) 2018 FMTech All Rights Reserved.
 *
 * @author Drew.Chiang
 * @version v1.0.0
 * @email chiangchuna@gmail.com
 * @create_date 2018/6/17 18:18
 * <p>
 * ==================================================================
 */

public class DownloadUtils {

    public static File download(String url, Context context){
        File file = null;
        InputStream is = null;
        FileOutputStream fos = null;
        try {
            file = new File(context.getCacheDir(), Constants.PATCH_FILE);
            if(file.exists()){
                file.delete();
            }
            HttpURLConnection connection = (HttpURLConnection)new URL(url).openConnection();
            connection.setDoInput(true);
            is = connection.getInputStream();
            fos = new FileOutputStream(file);
            byte[] buffer = new byte[512];
            int len = 0;
            while((len = is.read(buffer)) != -1){
                fos.write(buffer, 0, len);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if(null != fos){
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if(null != is){
                try {
                    is.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return file;
    }

}
