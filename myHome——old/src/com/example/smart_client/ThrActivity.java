package com.example.smart_client;




import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Timer;
import java.util.TimerTask;

import com.example.smart_client.GetxinxiActivity.MyHandler;
import com.fs.util.Single;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class ThrActivity extends Activity implements OnClickListener{

	Button  zaijia , lijia ,huijia , baojin;
	
	boolean sszaijia = true, sslijia, sshuijia, ssbaojin;
	
	private InputStream in = null;
	private BufferedInputStream bin = null;
	private OutputStream out = null;
	
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_thr);
		
		find();
		
		Single s = (Single) getApplicationContext();
		in = (InputStream) s.getValue("in");
		out = (OutputStream) s.getValue("out");
		bin = (BufferedInputStream) s.getValue("bin");
		
	}

	private void find() {
		// TODO Auto-generated method stub
	
		zaijia = (Button) findViewById(R.id.zaijia);
		lijia = (Button) findViewById(R.id.lijia);
		huijia = (Button) findViewById(R.id.huijia);
		baojin = (Button) findViewById(R.id.baojin);
		zaijia.setOnClickListener(this) ;	
		lijia.setOnClickListener(this);
		huijia.setOnClickListener(this);		
		baojin.setOnClickListener(this);
		//#################
	
	
		//��ȡ��ǰ״̬
	
	}
	
	public void onClick(View v) {
		String ss = null;
		switch (v.getId()) {
		//ȷ��
		
				
		case R.id.zaijia:

//        String ss =zaijia.getText().toString();
			if (sszaijia){	
				ss = "open_zj";				
				zaijia.setText("��");
				zaijia.setBackgroundResource(R.drawable.an3);
				sszaijia = false;
			}else{
				ss = "close_zj";
				zaijia.setText("��");
				zaijia.setBackgroundResource(R.drawable.an4);
				sszaijia = true;
			}
			break;
			
			
		case R.id.huijia:
			String gz = huijia.getText().toString();
			if (gz.equals("��")){
				ss = "open_hj";
				huijia.setText("��");
			}else{
				ss = "close_hj";
				huijia.setText("��");
			}
			break;
			
			
		case R.id.lijia:
			String fs = lijia.getText().toString();
			if (fs.equals("��")){
				ss = "open_lj";
				lijia.setText("��");
			}else{
				ss = "close_lj";
				lijia.setText("��");
			}
			break;
			
		case R.id.baojin:
			String fmq = baojin.getText().toString();
			if (fmq.equals("��")){
				ss = "open_bj";
				baojin.setText("��");
			}else{
				ss = "close_bj";
				baojin.setText("��");
			}
			
			break;
		}
		try {
			out.write(ss.getBytes());
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

}
	