package com.fmtech.incrementalupdate;

import android.os.Environment;

import java.io.File;

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

public class Constants {
    public static final String PATCH_FILE = "apk.patch";
    public static final String URL_PATCH_DOWNLOAD = "http://192.168.43.173:8080/"+PATCH_FILE;
    //linux remote


    public static final String SD_CARD = Environment.getExternalStorageDirectory() + File.separator;

    //new apk
    public static final String NEW_APK_PATH = SD_CARD+"cache/apk_new.apk";

    public static final String PATCH_FILE_PATH = SD_CARD+PATCH_FILE;
}
