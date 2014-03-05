package com.nmlzju.roaming3d;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.Queue;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import org.apache.http.util.EncodingUtils;
import org.json.JSONException;

import com.nmlzju.roaming3d.R;

import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.StrictMode;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;

@SuppressLint({ "NewApi", "NewApi", "NewApi" })
public class MainActivity extends Activity {

	private static final String TAG = "com.example.testcluster";
	private static final String IP_FILE = "/mnt/sdcard/TestCluster/TestCluster.txt";
	private static String IP = "192.168.137.1";
	private static final int PORT = 6666;
//	private static final int RECV_PORT = 6667;
//	private static final int DOWN = -1;
//	private static final int UP = -2;
	private static final int SEND_SIZE = 15;
	private static final int BUFFER_SIZE = 1024 * 8;
	private static final int BITMAP_SIZE = 960 * 540 * 4;
	
	private final int FLAG_MOVE = 1;
	private final int FLAG_SCALE = 0;
	private final int MOVE_DIS = 50 * 50;
	
	private GLSurfaceView mGLView;
	private Renderer renderer;
	private ImageView image;

	private int screen_width;
	private int screen_height;

	private float x0;
	private float y0;
	private float deltaX;
	private float deltaY;
	private float oldDistance;
	private float newDistance;
	private Queue<Packet> sendQueue = new LinkedList<Packet>();
	private int recvLength = 0;
	private byte[] recv = new byte[BUFFER_SIZE];
	private byte[] recvAll = new byte[BITMAP_SIZE];
//	private Bitmap recvBitmap;
	private recvThread rThread;
	private Socket socket;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Lock screen to landscape mode.
		
		this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

		// Set full screen, no Status-Bar or anything except the Activity
		// window!
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		// DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		screen_width = displayMetrics.widthPixels;
		screen_height = displayMetrics.heightPixels;

		setContentView(R.layout.activity_main);
		image = (ImageView) findViewById(R.id.image);
		//image.setImageResource(R.drawable.ic_action_search);
//		mGLView = new GLSurfaceView(getApplication());
//		renderer = new PicRender();
//		mGLView.setRenderer(renderer);

		
		StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()  
        .detectDiskReads()  
        .detectDiskWrites()  
        .detectNetwork()   // or .detectAll() for all detectable problems  
        .penaltyLog()  
        .build());  
StrictMode.setVmPolicy(new StrictMode.VmPolicy.Builder()  
        .detectLeakedSqlLiteObjects()  
        .detectLeakedClosableObjects()  
        .penaltyLog()  
        .penaltyDeath()  
        .build());  
		
//		File ipConfigFile = new File(IP_FILE);
//		if(ipConfigFile.exists()){
//			byte[] buffer = new byte[65];
//			 try {
//				FileInputStream fin = new FileInputStream(ipConfigFile);
//				fin.read(buffer);
//				IP = EncodingUtils.getString(buffer, "UTF-8");
//				fin.close();
//			} catch (FileNotFoundException e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//			} catch (IOException e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//				//fin.close();
//			}
//		}

		try {
			socket = new Socket(IP, PORT);
			new sendThread().start();
			rThread = new recvThread();
			rThread.start();
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		//setContentView(mGLView);
		
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		try{
			// TODO Auto-generated method stub
			// int historySize = event.getHistorySize();
			int pointCount = event.getPointerCount();
			if(pointCount > 2)
				pointCount = 2;
			float x = event.getX();
			float y = event.getY();
			switch (event.getAction() & MotionEvent.ACTION_MASK) {
			case MotionEvent.ACTION_DOWN:
				x0 = x;
				y0 = y;
				deltaX = deltaY = 0;
				Log.i(TAG, "Down " + x + "---" + y);
				break;
			case MotionEvent.ACTION_POINTER_DOWN:
				oldDistance = (event.getX(0) - event.getX(1)) * (event.getX(0) - event.getX(1)) + (event.getY(0) - event.getY(1)) * (event.getY(0) - event.getY(1));
				break;
			case MotionEvent.ACTION_CANCEL:
			case MotionEvent.ACTION_UP:
				Log.i(TAG, "UP " + x + "---" + y);
				break;
			case MotionEvent.ACTION_MOVE:
				if(pointCount == 1){
					deltaX = x - x0;
					deltaY = y - y0;
					x0 = x;
					y0 = y;
					if(deltaX < 2 && deltaX > -2)deltaX = 0;
					if(deltaY < 2 && deltaY > -2)deltaY = 0;
					
					Packet packet = new Packet(Packet.Command.MOVE);
					packet.args.put(deltaX);
					packet.args.put(deltaY);
					sendQueue.offer(packet);
	//				sendQueue.offer(new NetData(x, y));
					Log.i(TAG, "MOVE " + deltaX + "---" + deltaY);
				}else if(pointCount == 2){
					newDistance = (event.getX(0) - event.getX(1)) * (event.getX(0) - event.getX(1)) + (event.getY(0) - event.getY(1)) * (event.getY(0) - event.getY(1));
					Packet packet = new Packet(Packet.Command.SCALE);
					packet.args.put(newDistance - oldDistance);
					sendQueue.offer(packet);
					oldDistance = newDistance;
				}
				break;
			}
		}catch(JSONException e){
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return super.onTouchEvent(event);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

//	public class PicRender implements Renderer {
//
//		public void onDrawFrame(GL10 gl) {
//			// TODO Auto-generated method stub
//			
//		}
//
//		public void onSurfaceChanged(GL10 gl, int width, int height) {
//			// TODO Auto-generated method stub
//
//		}
//
//		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
//			// TODO Auto-generated method stub
//
//		}
//
//	}

	public class sendThread extends Thread {

		@Override
		public void run() {
			// TODO Auto-generated method stub
			super.run();
//			Socket socket;
			DataOutputStream dos = null;
			try {
//				socket = new Socket(IP, PORT);
				dos = new DataOutputStream(
						socket.getOutputStream());
				while (true) {
					if (sendQueue.isEmpty())
						continue;
					Packet packet = sendQueue.poll();
					StringBuilder sendString = new StringBuilder(packet.toString());
					sendString.append('\n');
					dos.write(sendString.toString().getBytes());
					dos.flush();
				}
//				dos.close();
			} catch (UnknownHostException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			try {
				dos.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

	}

	public Handler handler = new Handler() {
		
		@Override
		public void handleMessage(Message msg) {
		
			image.setImageBitmap((Bitmap)msg.obj);
		
		}
	};
	
	public class recvThread extends Thread{

		@Override
		public void run() {
			// TODO Auto-generated method stub
			super.run();
//			Socket socket;
			DataInputStream dis = null;
			Bitmap recvBitmap;
			int allLength = 0;
			int allInt = 0;
			try {
//				socket = new Socket(IP, RECV_PORT);
				dis = new DataInputStream(
						socket.getInputStream());
				while(true){
//					byte[] tmp = new byte[4];
//					dis.read(tmp);
//					Log.i(TAG, "jjjjjjjjj" + tmp);
					//dis.readFully(recv, 0, 4 * BITMAP_SIZE);
					//System.arraycopy(recv, 0, recvAll, 0, 4 * BITMAP_SIZE);
					
//					Arrays.fill(recvAll, 0);
//					for(int i = 0; i < 4 * BITMAP_SIZE; i++){
						//recvAll[i] = dis.readInt();
						
//					}
					
					
					allInt = dis.readInt();
					Log.i(TAG, "allInt " + allInt);
					allLength = 0;
					while (allLength < allInt) {
						if(allLength + BUFFER_SIZE <= allInt){
							dis.readFully(recv, 0, BUFFER_SIZE);
							recvLength = BUFFER_SIZE;
						}else{
							recvLength = allInt - allLength;
							dis.readFully(recv, 0, recvLength);
						}
						System.arraycopy(recv, 0, recvAll, allLength, recvLength);
						allLength += recvLength;
					}
					Log.i(TAG, "allLength " + allLength);
					
					recvBitmap = BitmapFactory.decodeByteArray(recvAll, 0, allLength);
					
					
//					recvBitmap = Bitmap.createBitmap(recvAll, 640, 380, Config.ARGB_8888);
					handler.obtainMessage(0, recvBitmap).sendToTarget();  //不能用handler.handleMessage(),
				}
//				dis.close();
//			} catch(Exception e){
//				
//			}
			} catch (UnknownHostException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			try {
				dis.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
	}



}
