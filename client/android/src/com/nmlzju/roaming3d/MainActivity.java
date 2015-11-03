package com.nmlzju.roaming3d;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.lang.reflect.Method;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketException;
import java.util.LinkedList;
import java.util.Queue;

import org.json.JSONArray;
import org.json.JSONException;

import com.nmlzju.roaming3d.R;

import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.*;
import android.preference.PreferenceManager;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.widget.ImageView;
import android.widget.Toast;

public class MainActivity extends Activity {

	private static String server_ip = null;
	private static int server_port = 0;
	private static int bitmap_size = 1;

	private ImageView scene;

	private int screen_width;
	private int screen_height;

	private float old_x;
	private float old_y;
	private float delta_x;
	private float delta_y;
	private float new_x;
	private float new_y;
	private float old_distance;
	private float new_distance;
	private int pointer_count = 0;
	
	private Queue<Packet> send_queue = new LinkedList<Packet>();
	private byte[] receive_buffer = null;
	private Socket socket = null;
	
	private Handler screen_handler = new ScreenHandler(this);
	private Handler toast_handler = new ToastHandler(this);
	private Callback[] callbacks = new Callback[Packet.Command.length.ordinal()];
	
	private boolean is_running = true;
	
	@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		//Get default settings
		SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(this);
		server_ip = settings.getString("server_ip", server_ip);
		try{
			server_port = Integer.parseInt(settings.getString("server_port", String.valueOf(server_port)));
		}catch(NumberFormatException e){
			Editor editor = settings.edit();
			editor.putString("server_port", "0");
			editor.apply();
		}
	
		//initialize callback functions
		callbacks[Packet.Command.UPDATE_VIDEO_FRAME.ordinal()] = new Callback(){
			public void handle(JSONArray args){
				try{
					int allLength = args.getInt(0);
					int recvLength = 0;
					
					DataInputStream stream = new DataInputStream(socket.getInputStream());
					while (recvLength < allLength) {
						recvLength += stream.read(receive_buffer, recvLength, allLength - recvLength);
					}
					
					Bitmap recvBitmap = BitmapFactory.decodeByteArray(receive_buffer, 0, allLength);
					screen_handler.obtainMessage(0, recvBitmap).sendToTarget(); // calling handler.handleMessage() or image.setBitmap() will crash
				} catch (JSONException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		};
		
		callbacks[Packet.Command.QUIT.ordinal()] = new Callback(){
			public void handle(JSONArray args){
				System.exit(0);
			}
		};
		
		callbacks[Packet.Command.MAKE_TOAST_TEXT.ordinal()] = new Callback(){
			public void handle(JSONArray args){
				String text = null;
				try {
					int toast_id = args.getInt(0);
					if(toast_id >= 0 && toast_id < Packet.Message2Toast.length){
						text = getString(Packet.Message2Toast[toast_id]);
					}
				} catch (JSONException e) {
					text = getString(R.string.toast_unkown_message);
				}
				
				if(text != null){
					toast_handler.obtainMessage(0, text).sendToTarget();
				}
			}
		};
		
		callbacks[Packet.Command.ENTER_HOTSPOT.ordinal()] = new Callback(){
			public void handle(JSONArray args){
				Intent intent = new Intent(MainActivity.this, HotspotActivity.class);
				intent.putExtra("hotspot", args.toString());
				startActivity(intent);
			}
		};
		
		//get the screen size
		Display display = getWindowManager().getDefaultDisplay();     
	    if(android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN){
	    	DisplayMetrics metrics = new DisplayMetrics();
	    	display.getRealMetrics(metrics);
            screen_width = metrics.widthPixels;
            screen_height = metrics.heightPixels;
        }else{
            try {
            	Method mGetRawH = Display.class.getMethod("getRawHeight");
            	Method mGetRawW = Display.class.getMethod("getRawWidth");
            	screen_width = (Integer) mGetRawW.invoke(display);
            	screen_height = (Integer) mGetRawH.invoke(display);
            } catch (Exception e) {
            	DisplayMetrics metrics = new DisplayMetrics();
    	    	display.getMetrics(metrics);
                screen_width = metrics.widthPixels;
                screen_height = metrics.heightPixels;
            }
        }
		
		bitmap_size = screen_width * screen_height * 4;
		receive_buffer = new byte[bitmap_size];

		setContentView(R.layout.activity_main);
		scene = (ImageView) findViewById(R.id.image);
		
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

		if(settings.getBoolean("auto_connect_on_start", true)){
			connectToServer();
		}
	}
	
	@Override
	protected void onDestroy() {
		is_running = false;
		
		if(socket != null){
			try {
				socket.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		
		super.onDestroy();
	}
	
	protected void connectToServer(){
		if(socket != null){
			return;
		}

		Thread connect_thread = new Thread(){	
			@Override
			public void run(){
				ConnectivityManager cmgr = (ConnectivityManager) getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
				if(cmgr == null){
					return;
				}
				NetworkInfo network = cmgr.getActiveNetworkInfo();
				if(network == null || !network.isConnected()){
					toast_handler.obtainMessage(0, getString(R.string.toast_network_not_available)).sendToTarget();
					return;
				}
				
				if(network.getType() != ConnectivityManager.TYPE_WIFI){
					toast_handler.obtainMessage(0, getString(R.string.toast_not_on_wifi)).sendToTarget();
				}
				
				if(server_port == 0 || server_ip == null || server_ip.isEmpty()){
					toast_handler.obtainMessage(0, getString(R.string.toast_detecting_server)).sendToTarget();
					
					DatagramSocket config_socket = null;
					try {
						config_socket = new DatagramSocket(null);
						config_socket.bind(new InetSocketAddress("0.0.0.0", 5260));
						config_socket.setBroadcast(true);
						config_socket.setSoTimeout(2000);
						byte[] data = new byte[]{0,0};
						InetAddress broadcast_address = InetAddress.getByAddress(new byte[]{(byte) 255, (byte) 255, (byte) 255, (byte) 255});
						DatagramPacket packet = new DatagramPacket(data, data.length, broadcast_address, 5261);
						config_socket.send(packet);
						config_socket.receive(packet);
						
						server_ip = packet.getAddress().getHostAddress();
						server_port = 0x0000FF00 & (data[1] << 8);
						server_port |= 0x000000FF & data[0];
						
						toast_handler.obtainMessage(0, getString(R.string.toast_detected_server) + " " + server_ip + ":" + server_port).sendToTarget();
					} catch (SocketException e) {
						e.printStackTrace();
						toast_handler.obtainMessage(0, getString(R.string.toast_socket_exception) + e.getLocalizedMessage()).sendToTarget();
					} catch (IOException e) {
						e.printStackTrace();
						toast_handler.obtainMessage(0, getString(R.string.toast_io_exception) + e.getLocalizedMessage()).sendToTarget();
					}
					
					if(config_socket != null){
						config_socket.close();
					}
					
					if(server_port == 0 || server_ip == null || server_ip.isEmpty()){
						return;
					}
					
					toast_handler.obtainMessage(0, getString(R.string.toast_start_connecting)).sendToTarget();
					try {
						socket = new Socket();
						socket.connect(new InetSocketAddress(server_ip, server_port), 3000);
						
						send_queue.clear();
						Packet packet = new Packet(Packet.Command.CREATE_DEVICE);
						packet.args.put(screen_width);
						packet.args.put(screen_height);
						send_queue.offer(packet);
					
						new SendThread().start();
						new ReceiveThread().start();
					} catch (Exception e) {
						toast_handler.obtainMessage(0, getString(R.string.toast_failed_to_connect_to_server)).sendToTarget();
						socket = null;
					}
				}
			}
		};
		
		connect_thread.start();
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {		
		new_x = event.getX();
		new_y = event.getY();

		switch (event.getAction() & MotionEvent.ACTION_MASK) {
		case MotionEvent.ACTION_UP:{
			float dis_x = Math.abs(old_x - new_x);
			float dis_y = Math.abs(old_y - new_y);
			if(dis_x < 1 && dis_y < 1 && event.getEventTime() - event.getDownTime() <= 100){
				onClick(event);
			}
		}
		
		case MotionEvent.ACTION_DOWN:{
			old_x = new_x;
			old_y = new_y;
			pointer_count = event.getPointerCount();
			break;
		}

		case MotionEvent.ACTION_POINTER_DOWN:{
			pointer_count = event.getPointerCount();
			
			if(pointer_count == 2){
				float disX = event.getX(0) - event.getX(1);
				float disY = event.getY(0) - event.getY(1);
				old_distance = disX * disX + disY * disY;
			}
			break;
		}

		case MotionEvent.ACTION_MOVE:
			if(pointer_count != event.getPointerCount()){
				break;
			}
			
			if (pointer_count == 1) {
				delta_x = new_x - old_x;
				delta_y = new_y - old_y;
				old_x = new_x;
				old_y = new_y;
				
				if(delta_x * delta_x + delta_y * delta_y < 8)
					break;

				Packet packet = new Packet(Packet.Command.ROTATE_CAMERA);
				packet.args.put((long)(delta_x * 100));
				packet.args.put((long)(delta_y * 100));
				send_queue.offer(packet);

			} else if (pointer_count == 2) {
				float disX = event.getX(0) - event.getX(1);
				float disY = event.getY(0) - event.getY(1);
				new_distance = disX * disX + disY * disY;
				float deltaDistance = (float) (Math.sqrt(new_distance) - Math.sqrt(old_distance));
				
				Packet packet = new Packet(Packet.Command.SCALE_CAMERA);
				packet.args.put((long)(deltaDistance * 100));
				send_queue.offer(packet);
				
				old_distance = new_distance;
			}
			
			break;
		}

		return super.onTouchEvent(event);
	}
	
	private long last_click_time = 0;
	public void onClick(MotionEvent e){
		if(e.getEventTime() - last_click_time <= 1000){
			onDoubleClick(e);
			last_click_time = 0;
		}
		last_click_time = e.getEventTime();
	}
	
	public void onDoubleClick(MotionEvent e){
		Packet packet = new Packet(Packet.Command.DOUBLE_CLICK);
		try {
			packet.args.put(e.getX());
			packet.args.put(e.getY());

			send_queue.offer(packet);
		} catch (JSONException exception) {
			Toast.makeText(this, exception.getMessage(), Toast.LENGTH_LONG).show();
		}
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
			send_queue.offer(packet);
			return true;
		case R.id.menu_settings:
			startActivity(new Intent(this, SettingsActivity.class));
			return true;
		case R.id.menu_exit:
			System.exit(0);
			return true;
		default:
			return super.onOptionsItemSelected(item);
	    }
	}

	public class SendThread extends Thread {
		@Override
		public void run() {
			if(socket == null){
				return;
			}
			
			DataOutputStream dos = null;
			try {
				dos = new DataOutputStream(socket.getOutputStream());
				while (is_running) {
					if (send_queue.isEmpty()){
						continue;
					}
					
					Packet packet = send_queue.poll();
					StringBuilder sendString = new StringBuilder(packet.toString());
					dos.writeBytes(sendString.toString());
					dos.writeByte('\n');
					dos.flush();
				}

			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public class ReceiveThread extends Thread {

		@Override
		public void run() {
			if(socket == null){
				return;
			}

			DataInputStream stream = null;
			int offset;
			try {
				stream = new DataInputStream(socket.getInputStream());
				while (is_running) {
					offset = 0;
					do{
						stream.readFully(receive_buffer, offset, 1);
					}while(offset < bitmap_size && receive_buffer[offset++] != '\n');
				
					String json = new String(receive_buffer, 0, offset);
					Packet packet = Packet.parse(json);
					if(packet == null){
						continue;
					}

					int command_id = packet.command.ordinal();
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
	
	static class ScreenHandler extends Handler{
		private final WeakReference<MainActivity> activity;
		
		public ScreenHandler(MainActivity activity){
			this.activity = new WeakReference<MainActivity>(activity);
		}
		
		public void handleMessage(Message msg){
			MainActivity activity = this.activity.get();
			Bitmap bitmap = (Bitmap) msg.obj;
			int width = activity.scene.getWidth();
			int height = activity.scene.getHeight();
			if (width != bitmap.getWidth() || height != bitmap.getHeight()) {
				bitmap = Bitmap.createScaledBitmap(bitmap, width, height, true);
			}
			activity.scene.setImageBitmap(bitmap);
		}
	}
	
	static class ToastHandler extends Handler{
		private final WeakReference<MainActivity> activity;
		private Toast toast = null;
		
		public ToastHandler(MainActivity activity){
			this.activity = new WeakReference<MainActivity>(activity);
		}
		
		public void handleMessage(Message msg){
			if(toast != null){
				toast.cancel();
			}
			
			MainActivity activity = this.activity.get();
			toast = Toast.makeText(activity, (String) msg.obj, Toast.LENGTH_LONG);
			toast.show();
		}
	}
}
