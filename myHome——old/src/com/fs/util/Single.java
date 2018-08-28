package com.fs.util;

import java.util.HashMap;
import java.util.Map;

import android.app.Application;

public class Single extends Application {

	// 定义一个集合，这个集合用来存放数据的，第一个参数表示键的类型，第二个参数表示值得类型，用的Object意味着可以装任意类型的数据
	private Map<String, Object> map = null;

	// 重写的方法，该方法在获得本类对象时会被自动调用
	@Override
	public void onCreate() {
		super.onCreate();
		// 方法内部实例化集合对象
		map = new HashMap<String, Object>();
		System.out.println("Single onCreate");
	}

	// 该方法用于存储数据，第一个参数标表示键，第二个参数表示值
	public void putValue(String key, Object value) {
		// 通过键值对的方式王Map集合中存数据
		map.put(key, value);
	}

	// 该方法用于根据键名取出数据，参数表示键名
	public Object getValue(String key) {
		// 通过键名取出数据，并返回一个Object对象
		Object O = map.get(key);
		return O;
	}
}