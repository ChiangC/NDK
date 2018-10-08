package com.fmtech.fmphix;

import android.content.Context;
import android.os.Build;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.Enumeration;

import dalvik.system.DexFile;

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
        if(null == file || !file.exists()){
            return;
        }

        try {
            DexFile dexFile = DexFile.loadDex(file.getAbsolutePath(), new File(mContext.getCacheDir(), "opt").getAbsolutePath(), Context.MODE_PRIVATE);
            if(null == dexFile){
                return;
            }
            Enumeration<String> entries = dexFile.entries();
            while(entries.hasMoreElements()){
                String className = entries.nextElement();
//                Class clazz = Class.forName(className);//Only available for loaded class;
                Class clazz = dexFile.loadClass(className, mContext.getClassLoader());
                if(null != clazz){
                    fixClazz(clazz);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void fixClazz(Class fixedClazz){
        if(null == fixedClazz){
            return;
        }
        Method[] methods = fixedClazz.getDeclaredMethods();
        for(Method rightMethod:methods){
            Replace replace = rightMethod.getAnnotation(Replace.class);
            if(null == replace){
                continue;
            }
            String wrongClassName = replace.clazz();
            String wrongMethodName = replace.method();
            try {
                Class clazz = Class.forName(wrongClassName);
                //Find the wrong method
                Method wrongMethod = clazz.getDeclaredMethod(wrongMethodName, rightMethod.getParameterTypes());
                if(Build.VERSION.SDK_INT <= 18){
                    replace(Build.VERSION.SDK_INT, wrongMethod, rightMethod);
                }
            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            }catch (NoSuchMethodException e) {
                e.printStackTrace();
            }
        }
    }

    public native void replace(int sdkVersionCode, Method wrongMethod, Method rightMethod);

}
