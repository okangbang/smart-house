package com.fs.util;

import java.util.HashMap;
import java.util.Map;

import android.app.Application;

public class Single extends Application {

	// ����һ�����ϣ������������������ݵģ���һ��������ʾ�������ͣ��ڶ���������ʾֵ�����ͣ��õ�Object��ζ�ſ���װ�������͵�����
	private Map<String, Object> map = null;

	// ��д�ķ������÷����ڻ�ñ������ʱ�ᱻ�Զ�����
	@Override
	public void onCreate() {
		super.onCreate();
		// �����ڲ�ʵ�������϶���
		map = new HashMap<String, Object>();
		System.out.println("Single onCreate");
	}

	// �÷������ڴ洢���ݣ���һ���������ʾ�����ڶ���������ʾֵ
	public void putValue(String key, Object value) {
		// ͨ����ֵ�Եķ�ʽ��Map�����д�����
		map.put(key, value);
	}

	// �÷������ڸ��ݼ���ȡ�����ݣ�������ʾ����
	public Object getValue(String key) {
		// ͨ������ȡ�����ݣ�������һ��Object����
		Object O = map.get(key);
		return O;
	}
}