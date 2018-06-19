package com.fmtech.fmphix;

import android.content.Context;

import java.io.File;

/**
 * ==================================================================
 * Copyright (C) 2018 FMTech All Rights Reserved.
 *
 * @author Drew.Chiang
 * @version v1.0.0
 * @email chiangchuna@gmail.com
 * @create_date 2018/6/19 16:50
 * <p>
 * ==================================================================
 */

public class DexManager {
    private static DexManager sInstance = new DexManager();
    private Context mContext;

    public static DexManager getInstance(){
        return sInstance;
    }

    public void setContext(Context context){
        mContext = context;
    }

    public void loadDexFile(File file){

    }


}
