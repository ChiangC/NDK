package com.fmtech.update;

public class IncrementalUpdate {
	public static void main(String[] args){
		ApkDiff.diff(ConstantsWin.OLD_APK_PATH, ConstantsWin.NEW_APK_PATH, ConstantsWin.PATCH_PATH);
	}
}
