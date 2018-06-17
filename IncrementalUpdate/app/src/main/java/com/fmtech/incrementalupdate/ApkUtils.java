package com.fmtech.incrementalupdate;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.support.v4.content.FileProvider;

import java.io.File;

/**
 * ==================================================================
 * Copyright (C) 2018 FMTech All Rights Reserved.
 *
 * @author Drew.Chiang
 * @version v1.0.0
 * @email chiangchuna@gmail.com
 * @create_date 2018/6/17 18:22
 * <p>
 * ==================================================================
 */

public class ApkUtils {
    public static int getVersionCode(Context context){
        PackageManager pm = context.getPackageManager();
        try {
            PackageInfo packageInfo = pm.getPackageInfo(context.getPackageName(), 0);
            return packageInfo.versionCode;
        } catch (Exception e) {
            e.printStackTrace();
            return 0;
        }
    }

    /**
     * /data/app/my.apk
     * @param context
     * @return
     */
    public static String getSourceApkPath(Context context){
        try {
            ApplicationInfo applicationInfo = context.getPackageManager()
                    .getApplicationInfo(context.getPackageName(), 0);
            return applicationInfo.sourceDir;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static void installApk(Context context, String apkPath){
        Intent intent = new Intent(Intent.ACTION_VIEW);
        File file = new File(apkPath);
        Uri apkUri = FileProvider.getUriForFile(context, "com.fmtech.incrementalupdate.fileprovider", file);
        intent.setDataAndType(Uri.parse("file://"+apkPath), "application/vnd.android.package-archive");
        context.startActivity(intent);
    }

}
