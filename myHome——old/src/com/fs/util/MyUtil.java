package com.fs.util;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import android.app.Application;

public class MyUtil {
	public static byte[]  read(BufferedInputStream bin, int size, int max) {
		byte[] image = new byte[size];
		int hasRead = 0;
		while (true) {
			if (max > size - hasRead) {
				max = size - hasRead;
			}
			try {
				hasRead = hasRead + bin.read(image, hasRead, max);
			} catch (IOException e) {
				e.printStackTrace();
			}
			if (hasRead == size) {
				break;
			}
		}
		return image;
	}
}



