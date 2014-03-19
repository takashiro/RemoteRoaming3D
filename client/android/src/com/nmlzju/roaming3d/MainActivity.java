package com.nmlzju.roaming3d;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.LinkedList;
import java.util.Queue;

import org.json.JSONArray;
import org.json.JSONException;

import com.nmlzju.roaming3d.R;

import android.os.*;
import android.preference.PreferenceManager;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.DisplayMetrics;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.Toast;

public class MainActivity extends Activity {

	private static String IP = "192.168.137.1";
	private static short PORT = 6666;
	private static int BITMAP_SIZE = 1;

	private ImageView image;

	private int screen_width;
	private int screen_height;

	private float oldX;
	private float oldY;
	private float deltaX;
	private float deltaY;
	private float newX;
	private float newY;
	private float oldDistance;
	private float newDistance;
	
	private Queue<Packet> sendQueue = new LinkedList<Packet>();
	private byte[] recvBuffer = null;
	private Socket socket = null;
	
	private Callback[] callbacks = new Callback[Packet.Command.length.ordinal()];

	private void connectToServer(){
		if(socket == null){
			Toast.makeText(this, getString(R.string.toast_start_connecting), Toast.LENGTH_LONG).show();
			try {
				socket = new Socket(IP, PORT);
				
				Packet packet = new Packet(Packet.Command.SET_RESOLUTION);
				packet.args.put(screen_width);
				packet.args.put(screen_height);
				sendQueue.offer(packet);
			
				new sendThread().start();
				new recvThread().start();
			} catch (UnknownHostException e) {
				Toast.makeText(this, getString(R.string.toast_failed_to_connect_to_server), Toast.LENGTH_LONG).show();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		// Set full screen, no Status-Bar or anything except the Activity window!
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

		super.onCreate(savedInstanceState);
		
		//Get default settings
		SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(this);
		IP = settings.getString("server_ip", IP);
		try{
			PORT = Short.parseShort(settings.getString("server_port", String.valueOf(PORT)));
		}catch(NumberFormatException e){
		}
	
		//initialize callback functions
		callbacks[Packet.Command.UPDATE_VIDEO_FRAME.ordinal()] = new Callback(){
			@Override
			public void handle(JSONArray args){
				try{
					int allLength = args.getInt(0);
					int recvLength = 0;
					
					DataInputStream stream = new DataInputStream(socket.getInputStream());
					while (recvLength < allLength) {
						recvLength += stream.read(recvBuffer, recvLength, allLength - recvLength);
					}
					
					Bitmap recvBitmap = BitmapFactory.decodeByteArray(recvBuffer, 0, allLength);
					handler.obtainMessage(0, recvBitmap).sendToTarget(); // calling handler.handleMessage() or image.setBitmap() will crash
				} catch (JSONException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		};
		
		callbacks[Packet.Command.QUIT.ordinal()] = new Callback(){
			@Override
			public void handle(JSONArray args){
				System.exit(0);
			}
		};
		
		//get the screen size
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		screen_width = displayMetrics.widthPixels;
		screen_height = displayMetrics.heightPixels;
		
		BITMAP_SIZE = screen_width * screen_height * 4;
		recvBuffer = new byte[BITMAP_SIZE];

		setContentView(R.layout.activity_main);
		image = (ImageView) findViewById(R.id.image);
		
		StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
			.detectDiskReads()
			.detectDiskWrites()
			.detectNetwork()
			.penaltyLog()
			.build()
		);
		StrictMode.setVmPolicy(new StrictMode.VmPolicy.Builder()
			.detectLeakedSqlLiteObjects()
			.detectLeakedClosableObjects()
			.penaltyLog()
			.penaltyDeath()
			.build()
		);
		
		if(settings.getBoolean("auto_connect_on_start", false)){
			connectToServer();
		}
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		try {
			int pointCount = event.getPointerCount();
			if (pointCount > 2) {
				pointCount = 2;
			}
			
			newX = event.getX();
			newY = event.getY();
	
			switch (event.getAction() & MotionEvent.ACTION_MASK) {
			case MotionEvent.ACTION_DOWN:
			case MotionEvent.ACTION_UP:{
				oldX = newX;
				oldY = newY;
				break;
			}
			
			case MotionEvent.ACTION_POINTER_DOWN:{
				float disX = event.getX(0) - event.getX(1);
				float disY = event.getY(0) - event.getY(1);
				oldDistance = disX * disX + disY * disY;
				break;
			}

			case MotionEvent.ACTION_MOVE:
				if (pointCount == 1) {
					deltaX = newX - oldX;
					deltaY = newY - oldY;
					oldX = newX;
					oldY = newY;
					
					if(deltaX * deltaX + deltaY * deltaY < 8)
						break;

					Packet packet = new Packet(Packet.Command.ROTATE_CAMERA);
					packet.args.put(deltaX);
					packet.args.put(deltaY);
					sendQueue.offer(packet);
					//Log.i(TAG, "MOVE " + deltaX + "---" + deltaY);
				} else if (pointCount == 2) {
					float disX = event.getX(0) - event.getX(1);
					float disY = event.getY(0) - event.getY(1);
					newDistance = disX * disX + disY * disY;
					float deltaDistance = (float) (Math.sqrt(newDistance) - Math.sqrt(oldDistance));
					
					Packet packet = new Packet(Packet.Command.SCALE_CAMERA);
					packet.args.put(deltaDistance);
					sendQueue.offer(packet);
					
					oldDistance = newDistance;
				}
				break;
			}
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return super.onTouchEvent(event);
	}

	@Override
	public void onDestroy() {
		if(socket != null){
			try {
				socket.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		super.onDestroy();
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.menu_connect:
			connectToServer();
			return true;
		case R.id.menu_hotspot:
			Packet packet = new Packet(Packet.Command.CONTROL_HOTSPOTS);
			sendQueue.offer(packet);
			return true;
		case R.id.menu_settings:
			startActivity(new Intent(MainActivity.this, SettingsActivity.class));
			return true;
		case R.id.menu_exit:
			System.exit(0);
			return true;
		default:
			return super.onOptionsItemSelected(item);
	    }
	}

	public class sendThread extends Thread {
		@Override
		public void run() {
			DataOutputStream dos = null;
			try {
				dos = new DataOutputStream(socket.getOutputStream());
				while (true) {
					if (sendQueue.isEmpty())
						continue;
					Packet packet = sendQueue.poll();
					StringBuilder sendString = new StringBuilder(packet.toString());
					dos.write(sendString.toString().getBytes());
					dos.flush();
				}

			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			if(dos != null){
				try {
					dos.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
	}

	private Handler handler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			image.setImageBitmap((Bitmap) msg.obj);
		}
	};

	public class recvThread extends Thread {

		@Override
		public void run() {
			super.run();

			DataInputStream stream = null;
			int offset;
			try {
				stream = new DataInputStream(socket.getInputStream());
				while (true) {
					offset = 0;
					do{
						stream.readFully(recvBuffer, offset, 1);
					}while(offset < BITMAP_SIZE && recvBuffer[offset++] != '\n');
				
					String json = new String(recvBuffer, 0, offset);
					Packet packet = Packet.parse(json);
					if(packet == null){
						continue;
					}

					int command_id = 1;//packet.command.ordinal();
					if(command_id < callbacks.length){
						Callback func = callbacks[command_id];
						if(func != null){
							func.handle(packet.args);
						}
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}
