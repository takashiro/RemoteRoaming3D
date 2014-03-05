package com.nmlzju.roaming3d;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.LinkedList;
import java.util.Queue;

import org.json.JSONException;

import com.nmlzju.roaming3d.R;

import android.os.*;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;

@SuppressLint({ "NewApi", "NewApi", "NewApi" })
public class MainActivity extends Activity {

	private static final String TAG = "com.nlmzju.roaming3d";
	private static String IP = "192.168.137.1";
	private static final int PORT = 6666;
	private static final int BUFFER_SIZE = 1024 * 8;
	private static final int BITMAP_SIZE = 960 * 540 * 4;

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
	// private Bitmap recvBitmap;
	private Socket socket;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		// Set full screen, no Status-Bar or anything except the Activity window!
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		screen_width = displayMetrics.widthPixels;
		screen_height = displayMetrics.heightPixels;

		setContentView(R.layout.activity_main);
		image = (ImageView) findViewById(R.id.image);
		
		StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
			.detectDiskReads()
			.detectDiskWrites()
			.detectNetwork()
			.penaltyLog()
			.build()
		);
		StrictMode.setVmPolicy(new StrictMode.VmPolicy.Builder().detectLeakedSqlLiteObjects().detectLeakedClosableObjects().penaltyLog().penaltyDeath().build());
		
		try {
			socket = new Socket(IP, PORT);

			Packet packet = new Packet(Packet.Command.SET_RESOLUTION);
			packet.args.put(screen_width);
			packet.args.put(screen_height);
			sendQueue.offer(packet);

			new sendThread().start();
			new recvThread().start();
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		try {
			// int historySize = event.getHistorySize();
			int pointCount = event.getPointerCount();
			if (pointCount > 2) {
				pointCount = 2;
			}
			
			float x = event.getX();
			float y = event.getY();
			switch (event.getAction() & MotionEvent.ACTION_MASK) {
			case MotionEvent.ACTION_DOWN:
				x0 = x;
				y0 = y;
				deltaX = deltaY = 0;
				//Log.i(TAG, "Down " + x + "---" + y);
				break;
			case MotionEvent.ACTION_POINTER_DOWN:
				oldDistance = (event.getX(0) - event.getX(1)) * (event.getX(0) - event.getX(1)) + (event.getY(0) - event.getY(1)) * (event.getY(0) - event.getY(1));
				break;
			case MotionEvent.ACTION_CANCEL:
			case MotionEvent.ACTION_UP:
				//Log.i(TAG, "UP " + x + "---" + y);
				break;
			case MotionEvent.ACTION_MOVE:
				if (pointCount == 1) {
					deltaX = x - x0;
					deltaY = y - y0;
					x0 = x;
					y0 = y;
					if (deltaX < 2 && deltaX > -2)
						deltaX = 0;
					if (deltaY < 2 && deltaY > -2)
						deltaY = 0;

					Packet packet = new Packet(Packet.Command.MOVE);
					packet.args.put(deltaX);
					packet.args.put(deltaY);
					sendQueue.offer(packet);
					//Log.i(TAG, "MOVE " + deltaX + "---" + deltaY);
				} else if (pointCount == 2) {
					newDistance = (event.getX(0) - event.getX(1)) * (event.getX(0) - event.getX(1)) + (event.getY(0) - event.getY(1)) * (event.getY(0) - event.getY(1));
					Packet packet = new Packet(Packet.Command.SCALE);
					packet.args.put(newDistance - oldDistance);
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
					sendString.append('\n');
					dos.write(sendString.toString().getBytes());
					dos.flush();
				}

			} catch (UnknownHostException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
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

			DataInputStream dis = null;
			Bitmap recvBitmap;
			int allLength = 0;
			int allInt = 0;
			try {
				dis = new DataInputStream(socket.getInputStream());
				while (true) {
					allInt = dis.readInt();
					//Log.i(TAG, "allInt " + allInt);
					allLength = 0;
					while (allLength < allInt) {
						if (allLength + BUFFER_SIZE <= allInt) {
							dis.readFully(recv, 0, BUFFER_SIZE);
							recvLength = BUFFER_SIZE;
						} else {
							recvLength = allInt - allLength;
							dis.readFully(recv, 0, recvLength);
						}
						System.arraycopy(recv, 0, recvAll, allLength, recvLength);
						allLength += recvLength;
					}
					//Log.i(TAG, "allLength " + allLength);

					recvBitmap = BitmapFactory.decodeByteArray(recvAll, 0, allLength);

					handler.obtainMessage(0, recvBitmap).sendToTarget(); // calling handler.handleMessage() or image.setBitmap() will crash
				}
				// dis.close();
				// } catch(Exception e){
				//
				// }
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
