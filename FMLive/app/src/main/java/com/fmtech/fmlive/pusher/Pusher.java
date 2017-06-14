/**
 *==================================================================
 * Copyright (C) 2017 FMTech All Rights Reserved.
 *
 * @author Drew.Chiang
 *
 * @email chiangchuna@gmail.com
 *
 * @version v1.0.0
 *
 * @create_date 2017年3月18日 下午9:26:39
 *
 *==================================================================
 */
package com.fmtech.fmlive.pusher;

public abstract class Pusher {
	public abstract void startPush();

	public abstract void stopPush();

	public abstract void release();
}
