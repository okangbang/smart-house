package com.example.smart_client;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;

import com.fs.util.Single;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class MainActivity extends Activity {

	EditText name, mima;
	Button denglu, zhuce;

	String IP = "192.168.3.243";
	int PORT = 9999;
	Socket socket = null;
	InputStream in = null;
	OutputStream out = null;
	BufferedInputStream bin = null;
	
	
	String yonghuming;
	String pass;

	Handler handler;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		find();

		new MyThread().start();
		
		// 7、实例化Handler对象
		handler = new MyHandler();
	}

	// 8、在主配置文件中红配置权限
	// <uses-permission android:name="android.permission.INTERNET"/>

	private void find() {
		name = (EditText) findViewById(R.id.name);
		mima = (EditText) findViewById(R.id.mima);

		zhuce = (Button) findViewById(R.id.zhuce);
		zhuce.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				new MyThread1().start();
			}
		});

		denglu = (Button) findViewById(R.id.denglu);
		denglu.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				Log.i("Main", "点击了登陆按钮");

				// （1）取出编辑框中的用户名和密码
				yonghuming = name.getText().toString();
				pass = mima.getText().toString();

				Log.i("Main", "取出用户名和密码" + yonghuming + "&" + pass);

				// 1、开启子线程连接服务器
				new MyThread2().start();
				Log.i("Main", "开启子线程收发数据");
			}
		});
	}

	class MyThread extends Thread {
		@Override
		public void run() {
			super.run();
			// 2、在子线程中创建Socket连接服务器
			try {
				if (socket == null) {// 判断socket是否为null，如果为null，则去连接，如果不为null，可以直接使用之前的socket，并不会重复连接
					// 创建Socket对象，并且绑定服务器的ip地址和端口号
					socket = new Socket(IP, PORT);
					// 获取输入输出流对象
					in = socket.getInputStream();
					out = socket.getOutputStream();
					bin = new BufferedInputStream(in);
					
					Single s = (Single) getApplicationContext();
					s.putValue("socket", socket);
					s.putValue("in", in);
					s.putValue("out", out);
					s.putValue("bin", bin);
				
				}
				
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
	class MyThread2 extends Thread {
		@Override
		public void run() {
			super.run();

			// 2、在子线程中创建Socket连接服务器
			try {
			
				// 3、通过输出流对象向服务器发送用户名和密码进行验证

				// （2）将取出的用户名和密码连接成固定的格式（协议）
				String data = "d" + "#" + yonghuming + ":" + pass + "end";// "abc#123end"
				Log.i("Main", "发送用户名的密码" + data);

				// （3）发送数据
				out.write(data.getBytes());

				// 4、读取验证信息
				// （1）准备一个字节数组，用来存放接收到的数据
				byte[] bufin = new byte[128];
				// (2)读取数据
				in.read(bufin);
				Log.i("Main", "读取验证信息");

				// 5、将读取到的验证信息发送到主线程进行处理
				Message msg = new Message();// 创建一个Message对象
				msg.what = 1;
				msg.obj = bufin;// 将验证信息添加到Message对象中
				handler.sendMessage(msg);// 通过Handler对象发送消息
				Log.i("Main", "发送验证信息到主线程");

			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	class MyThread1 extends Thread {
		@Override
		public void run() {
//			super.run();

			// 2、在子线程中创建Socket连接服务器
			try {
		
				// 3、通过输出流对象向服务器发送用户名和密码进行验证
				// （1）取出编辑框中的用户名和密码
				String yonghuming = name.getText().toString();
				String pass = mima.getText().toString();
				// （2）将取出的用户名和密码连接成固定的格式（协议）
				String data = "z" + "#" + yonghuming + ":" + pass + "end";// "abc#123end"
				// （3）发送数据
				out.write(data.getBytes());

				// 4、读取验证信息
				// （1）准备一个字节数组，用来存放接收到的数据
				byte[] bufin = new byte[128];
				// (2)读取数据
				in.read(bufin);

				// 5、将读取到的验证信息发送到主线程进行处理
				Message msg = new Message();// 创建一个Message对象
				msg.obj = bufin;// 将验证信息添加到Message对象中
				handler.sendMessage(msg);// 通过Handler对象发送消息

			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	// 6、在主线程中处理消息
	class MyHandler extends Handler {
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			int what = msg.what;

			if (what == 1) {// 登陆
				Log.i("Main", "what == 1");

				// 取出message对象中的内容
				byte[] buf = (byte[]) msg.obj;
				// 将字节数组转为字符串格式
				String s = new String(buf);
				// 将字符串前后的空格去掉
				String str = s.substring(0, 2);
				Log.i("Main", "验证信息是：" + str);

				// 进行验证
				if (str.equals("OK")) {// 验证成功，则进行界面跳转
					Toast.makeText(MainActivity.this, "验证成功",
							Toast.LENGTH_SHORT).show();

					Intent intent = new Intent();
					intent.setClass(MainActivity.this, TwoActivity.class);
					startActivity(intent);

				} else if (str.equals("FALSE")) {// 验证失败，则进行友好提示
					Toast.makeText(MainActivity.this, "验证失败",
							Toast.LENGTH_SHORT).show();

				}

			} else if (what == 2) {// 注册

			}

		}
	}
}
