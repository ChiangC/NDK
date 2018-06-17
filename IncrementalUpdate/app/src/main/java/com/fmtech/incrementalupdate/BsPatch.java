package com.fmtech.incrementalupdate;

/**
 * ==================================================================
 * Copyright (C) 2018 FMTech All Rights Reserved.
 *
 * @author Drew.Chiang
 * @version v1.0.0
 * @email chiangchuna@gmail.com
 * @create_date 2018/6/17 20:42
 * <p>
 * ==================================================================
 */

public class BsPatch {
    static {
        System.loadLibrary("BsPatch");
    }

    public native static int patch(String oldfile, String newFile, String patchFile);


}
