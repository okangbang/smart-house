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
		
		// 7��ʵ����Handler����
		handler = new MyHandler();
	}

	// 8�����������ļ��к�����Ȩ��
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
				Log.i("Main", "����˵�½��ť");

				// ��1��ȡ���༭���е��û���������
				yonghuming = name.getText().toString();
				pass = mima.getText().toString();

				Log.i("Main", "ȡ���û���������" + yonghuming + "&" + pass);

				// 1���������߳����ӷ�����
				new MyThread2().start();
				Log.i("Main", "�������߳��շ�����");
			}
		});
	}

	class MyThread extends Thread {
		@Override
		public void run() {
			super.run();
			// 2�������߳��д���Socket���ӷ�����
			try {
				if (socket == null) {// �ж�socket�Ƿ�Ϊnull�����Ϊnull����ȥ���ӣ������Ϊnull������ֱ��ʹ��֮ǰ��socket���������ظ�����
					// ����Socket���󣬲��Ұ󶨷�������ip��ַ�Ͷ˿ں�
					socket = new Socket(IP, PORT);
					// ��ȡ�������������
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

			// 2�������߳��д���Socket���ӷ�����
			try {
			
				// 3��ͨ�����������������������û��������������֤

				// ��2����ȡ�����û������������ӳɹ̶��ĸ�ʽ��Э�飩
				String data = "d" + "#" + yonghuming + ":" + pass + "end";// "abc#123end"
				Log.i("Main", "�����û���������" + data);

				// ��3����������
				out.write(data.getBytes());

				// 4����ȡ��֤��Ϣ
				// ��1��׼��һ���ֽ����飬������Ž��յ�������
				byte[] bufin = new byte[128];
				// (2)��ȡ����
				in.read(bufin);
				Log.i("Main", "��ȡ��֤��Ϣ");

				// 5������ȡ������֤��Ϣ���͵����߳̽��д���
				Message msg = new Message();// ����һ��Message����
				msg.what = 1;
				msg.obj = bufin;// ����֤��Ϣ��ӵ�Message������
				handler.sendMessage(msg);// ͨ��Handler��������Ϣ
				Log.i("Main", "������֤��Ϣ�����߳�");

			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	class MyThread1 extends Thread {
		@Override
		public void run() {
//			super.run();

			// 2�������߳��д���Socket���ӷ�����
			try {
		
				// 3��ͨ�����������������������û��������������֤
				// ��1��ȡ���༭���е��û���������
				String yonghuming = name.getText().toString();
				String pass = mima.getText().toString();
				// ��2����ȡ�����û������������ӳɹ̶��ĸ�ʽ��Э�飩
				String data = "z" + "#" + yonghuming + ":" + pass + "end";// "abc#123end"
				// ��3����������
				out.write(data.getBytes());

				// 4����ȡ��֤��Ϣ
				// ��1��׼��һ���ֽ����飬������Ž��յ�������
				byte[] bufin = new byte[128];
				// (2)��ȡ����
				in.read(bufin);

				// 5������ȡ������֤��Ϣ���͵����߳̽��д���
				Message msg = new Message();// ����һ��Message����
				msg.obj = bufin;// ����֤��Ϣ��ӵ�Message������
				handler.sendMessage(msg);// ͨ��Handler��������Ϣ

			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	// 6�������߳��д�����Ϣ
	class MyHandler extends Handler {
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			int what = msg.what;

			if (what == 1) {// ��½
				Log.i("Main", "what == 1");

				// ȡ��message�����е�����
				byte[] buf = (byte[]) msg.obj;
				// ���ֽ�����תΪ�ַ�����ʽ
				String s = new String(buf);
				// ���ַ���ǰ��Ŀո�ȥ��
				String str = s.substring(0, 2);
				Log.i("Main", "��֤��Ϣ�ǣ�" + str);

				// ������֤
				if (str.equals("OK")) {// ��֤�ɹ�������н�����ת
					Toast.makeText(MainActivity.this, "��֤�ɹ�",
							Toast.LENGTH_SHORT).show();

					Intent intent = new Intent();
					intent.setClass(MainActivity.this, TwoActivity.class);
					startActivity(intent);

				} else if (str.equals("FALSE")) {// ��֤ʧ�ܣ�������Ѻ���ʾ
					Toast.makeText(MainActivity.this, "��֤ʧ��",
							Toast.LENGTH_SHORT).show();

				}

			} else if (what == 2) {// ע��

			}

		}
	}
}
