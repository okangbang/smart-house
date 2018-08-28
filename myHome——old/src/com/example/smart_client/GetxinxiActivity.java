package com.example.smart_client;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;

import com.fs.util.MyUtil;
import com.fs.util.Single;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class GetxinxiActivity extends Activity {

	private TextView liangd, wend, chuangl,dians;
	private Button button1; //l,LL,w,WW,t,TT,c,CC;

	private Timer timer;
	private Handler handler;

	private InputStream in = null;
	private BufferedInputStream bin = null;
	private OutputStream out = null;

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_getxinxi);

		find();

		Single s = (Single) getApplicationContext();
		in = (InputStream) s.getValue("in");
		out = (OutputStream) s.getValue("out");
		bin = (BufferedInputStream) s.getValue("bin");

		timer = new Timer();
		handler = new MyHandler();
	}

	private void find() {
		liangd = (TextView) findViewById(R.id.liangd);
		wend = (TextView) findViewById(R.id.wend);
		chuangl = (TextView) findViewById(R.id.chuangl);
		dians = (TextView) findViewById(R.id.dians);

	
		button1 = (Button) findViewById(R.id.button1);
		button1.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {

				timer.schedule(new TimerTask() {

					@Override
					public void run() {

						try {

							String str = "get_env";
							out.write(str.getBytes());

							byte[] buf = new byte[20];
							in.read(buf);
							String data = new String(buf);
							
							Message msg = new Message();
							msg.obj = data;
							handler.sendMessage(msg);

						} catch (IOException e) {
							e.printStackTrace();
						}
					}
				}, 200, 3000);
			}
		});
	}

	protected void onPause() {
		super.onPause();
		try {
			String s = "stop_x";
			out.write(s.getBytes());
			this.timer.cancel();

		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	// 6、在主线程中处理消息
	class MyHandler extends Handler {
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			// 取出message对象中的内容
			String data = (String) msg.obj;
			// 将字符串前后的空格去掉
			String str = data.trim();
			//显示亮度
			String l = str.substring(str.indexOf("env") + 3,
					str.indexOf("light"));
			liangd.setText(l);
			//显示温度
			String h = str.substring(str.indexOf("light") + 5,  
					str.indexOf("temp")); 
			wend.setText(h);
			//显示电视
			String t = str.substring(str.indexOf("chuang") + 6, //hum
					str.indexOf("tv"));
			dians.setText(t);
			//显示窗帘
			String c = str.substring(str.indexOf("temp") + 4, //hum
					str.indexOf("tv")); //light
			chuangl.setText(c);
		}
	}

	public byte[] read(BufferedInputStream bin, int size, int max) {
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