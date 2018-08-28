package com.example.smart_client;


import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;


public class TwoActivity extends Activity implements OnClickListener{

	Button  picture,mos,huanjing;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_two);
		
		find();
	}

	private void find() {
		mos = (Button) findViewById(R.id.mos);
		picture = (Button) findViewById(R.id.picture);
		huanjing = (Button) findViewById(R.id.huanjing);
		
		mos.setOnClickListener(this);
		picture.setOnClickListener(this);
		huanjing.setOnClickListener(this); 
		}

	@Override
	public void onClick(View v) {
		
		switch (v.getId()) {
			
		case R.id.mos:
			//参数设置界面跳转
			Intent intent1 = new Intent();
			intent1.setClass(TwoActivity.this,ThrActivity.class);
			startActivity(intent1);
			break;
			
			
			
		case R.id.picture:			
			//参数设置界面跳转
			Intent intent2 = new Intent();
			intent2.setClass(TwoActivity.this,ShipinActivity.class);
			startActivity(intent2);					
			break;
			
			
		case R.id.huanjing:
			
			//参数设置界面跳转
			Intent intent3 = new Intent();
			intent3.setClass(TwoActivity.this,GetxinxiActivity.class);
			startActivity(intent3);
			 
			break;
		}	
	}
}
	