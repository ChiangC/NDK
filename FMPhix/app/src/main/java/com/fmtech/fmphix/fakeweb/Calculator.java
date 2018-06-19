package com.fmtech.fmphix.fakeweb;

import com.fmtech.fmphix.Replace;

/**
 * ==================================================================
 * Copyright (C) 2018 FMTech All Rights Reserved.
 *
 * @author Drew.Chiang
 * @version v1.0.0
 * @email chiangchuna@gmail.com
 * @create_date 2018/6/19 16:57
 * <p>
 * ==================================================================
 */

public class Calculator {
    @Replace(clazz = "com.fmtech.fmphix.Calculator", method = "calculate")
    public int calculate(){
        int i = 100;
        int j = 10;
        return i/j;
    }
}
