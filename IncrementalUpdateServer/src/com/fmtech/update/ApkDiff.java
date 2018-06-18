package com.fmtech.update;

public class ApkDiff {
	static{
		System.loadLibrary("ApkDiff");
	}
	
	public static native void diff(String oldApkPath, String newApkPath, String patchPath);

}
