package com.example.smart_client;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import com.fs.util.MyUtil;
import com.fs.util.Single;

public class ShipinActivity extends Activity {


	private ImageView imageView;
	private Button open, close;

	private Timer timer;
	private Handler handler;

	private Socket socket = null;
	private InputStream in = null;
	private BufferedInputStream bin = null;
	private OutputStream out = null;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		this.setContentView(R.layout.activity_shipin);

		this.init();

		Single s = (Single) getApplicationContext();
		in = (InputStream) s.getValue("in");
		out = (OutputStream) s.getValue("out");
		bin = (BufferedInputStream) s.getValue("bin");

		this.handler = new MyHandler();// UI
		this.timer = new Timer();
	}

	private void init() {
		this.imageView = (ImageView) this.findViewById(R.id.imageView);
		this.open = (Button) this.findViewById(R.id.open);
		this.close = (Button) this.findViewById(R.id.close);

		open.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				open();
			}
		});

		close.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				close();
			}
		});
	}

	public void open() {
		Toast.makeText(this, "start", 1).show();

		this.timer.schedule(new TimerTask() {

			@Override
			public void run() {// 子
				System.out.println("run....");
				try {

					String str = "get_img";
					out.write(str.getBytes());

					byte[] len1 = MyUtil.read(bin, 20, 4096);// 读包含长度的20个字节
					String len2 = new String(len1); // "1423len.....0"
					len2 = len2.substring(0, len2.indexOf("len"));// "1423"
					int len3 = new Integer(len2);// 1423

					byte[] image = MyUtil.read(bin, len3, 4096);// 读取图片

					Message msg = new Message();
					msg.what = 1;
					msg.obj = image;
					handler.sendMessage(msg);

				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}, 500, 200);
	}

	public void close() {
		try {
			
			out.write("stop_imge".getBytes());
			this.timer.cancel();
			timer = new Timer();
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	protected void onPause() {
		super.onPause();
		try {
			
			out.write("stop_imge".getBytes());
			this.timer.cancel();
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	class MyHandler extends Handler {
		private Bitmap bitmap;

		@Override
		public void handleMessage(Message msg) {// UI
			int what = msg.what;// 1
			if (what == 1) {
				byte[] image = (byte[]) msg.obj;
				if (this.bitmap != null && !this.bitmap.isRecycled()) {
					this.bitmap.recycle();
				}
				this.bitmap = BitmapFactory.decodeByteArray(image, 0,
						image.length);
				imageView.setImageBitmap(bitmap);
			}
		}
	}
}
