package com.example.testcluster;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.net.Uri;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

public class MyCameraActivity extends Activity {
    /** Called when the activity is first created. */
	
	private CameraView cv;
	private Camera mCamera = null;
	private Bitmap mBitmap = null;
//	private SurfaceView mSurfaceView;
	private int mScreenWidth;
	private int mScreenHeight;
	private TelephonyManager tm;
	private String  phoneid;
	//private FrameLayout fl;
	
	   /**
     * ͼƬȥɫ,���ػҶ�ͼƬ
    * @param bmpOriginal �����ͼƬ
    * @return ȥɫ���ͼƬ
    */
   public static Bitmap toGrayscale(Bitmap bmpOriginal) {
       int width, height;
       height = bmpOriginal.getHeight();
       width = bmpOriginal.getWidth();    

       Bitmap bmpGrayscale = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
       Canvas c = new Canvas(bmpGrayscale);
       Paint paint = new Paint();
       ColorMatrix cm = new ColorMatrix();
       cm.setSaturation(0);
       ColorMatrixColorFilter f = new ColorMatrixColorFilter(cm);
       paint.setColorFilter(f);
       c.drawBitmap(bmpOriginal, 0, 0, paint);
       return bmpGrayscale;
   }
	
	public Camera.PictureCallback pictureCallback = new Camera.PictureCallback() {
		
		public void onPictureTaken(byte[] data, Camera camera) {
			// TODO Auto-generated method stub
			Log.i("ygy", "onPictureTaken");
			Toast.makeText(getApplicationContext(), "���ڱ���...", Toast.LENGTH_LONG).show();
			mBitmap = MyCameraActivity.toGrayscale(BitmapFactory.decodeByteArray(data, 0, data.length));
			String path = "/sdcard/OurVoice/"+phoneid+".jpg";
			File file = new File(path);
			try {
				file.createNewFile();
				BufferedOutputStream os = new BufferedOutputStream(new FileOutputStream(file));
				mBitmap.compress(Bitmap.CompressFormat.JPEG, 20, os);
				os.flush();
				os.close();
				Toast.makeText(getApplicationContext(), "ͼ�񱣴�ɹ�", Toast.LENGTH_LONG).show();

			    String tmpname = FileUpload.uploadFile(path, "upload","0");
//			    Uri uri = Uri.fromFile(new File(path));
//   			    Intent intent = new Intent();
//   			    intent.setAction("android.intent.action.VIEW");
//   			    intent.setDataAndType(uri, "image/jpeg");
//   			    startActivity(intent);
				Intent intent = new Intent(MyCameraActivity.this,MainActivity.class);
				intent.putExtra("name", tmpname);
				startActivity(intent);
   			    
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
	};
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        tm = (TelephonyManager) this.getSystemService(TELEPHONY_SERVICE);   
        phoneid=tm.getDeviceId();//
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        
        setContentView(R.layout.capture);
        //SurfaceView mSurfaceView;
     //   mSurfaceView = (SurfaceView) findViewById(R.id.surface_camera);
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        mScreenWidth = dm.widthPixels;
        mScreenHeight = dm.heightPixels;
        Log.i("ping", mScreenWidth+"");
        Log.i("ping", mScreenHeight+"");
        
        getWindow().setFormat(PixelFormat.TRANSLUCENT);
        cv = new CameraView(this);
        
       /* fl = new FrameLayout(this);        
        fl.addView(cv);
        
        TextView tv = new TextView(this);
        tv.setText("������");
        fl.addView(tv);
       */
        setContentView(cv);
    }
    
    
    
    
    /* (non-Javadoc)
	 * @see android.app.Activity#onKeyDown(int, android.view.KeyEvent)
	 */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		
		Log.i("ygy", "onKeyDown");
		if(keyCode==KeyEvent.KEYCODE_MENU) {
			if(mCamera!=null) {
				Log.i("ygy", "mCamera.takePicture");
				//mCamera.autoFocus(null);
				mCamera.takePicture(null, null, pictureCallback);
			}
		}
		
		return super.onKeyDown(keyCode, event);
	}




	class CameraView extends SurfaceView {
//	class CameraView extends SurfaceView implements SurfaceHolder.Callback{
    	private SurfaceHolder holder = null;

		public CameraView(Context context) {
			super(context);
			// TODO Auto-generated constructor stub
			
			Log.i("ygy", "CameraView");
			holder = this.getHolder();
			holder.addCallback(new SurfaceHolder.Callback() {
				
				public void surfaceDestroyed(SurfaceHolder holder) {
					// TODO Auto-generated method stub
					mCamera.stopPreview();
					mCamera.release();
					mCamera = null;
				}
				
				public void surfaceCreated(SurfaceHolder holder) {
					// TODO Auto-generated method stub
					mCamera = Camera.open();
					try {
						//mCamera.setPreviewDisplay(holder);
						
			            Camera.Parameters param = mCamera.getParameters();  
			            param.setPictureFormat(PixelFormat.JPEG);
						int bestWidth = 1024;
						int bestHeight = 600;
						List<Camera.Size> sizeList = param.getSupportedPreviewSizes();
						//���sizeListֻ��һ������Ҳû�б�Ҫ��ʲô�ˣ���Ϊ����һ������ѡ��
						String ssize=String.valueOf(sizeList.size());
						Log.i("surfaceCreated.............................................", ssize);
						if(sizeList.size() > 1){
							Iterator<Camera.Size> itor = sizeList.iterator();
							while(itor.hasNext()){
								Camera.Size cur = itor.next();
								String swidth=String.valueOf(cur.width);
								String sheight=String.valueOf(cur.height);
								Log.i("surfaceChanged.............................................", swidth);
								Log.i("surfaceChanged.............................................", sheight);
								if(cur.width < bestWidth || cur.height < bestHeight ){
									bestWidth = cur.width;
									bestHeight = cur.height;
								}
							}
							//if(bestWidth!=1024){
							//	param.setPreviewSize(bestWidth, bestHeight);
								param.setPictureSize(bestWidth, bestHeight);
								//����ı���SIze�����ǻ�Ҫ����SurfaceView������Surface������ı��С������Camera��ͼ�������ܲ�
								//cv.setLayoutParams(new LinearLayout.LayoutParams(bestWidth, bestHeight));
							//fl.setLayoutParams(new LayoutParams(bestWidth, bestHeight));
							//}
								/*
								RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) cv.getLayoutParams();
								lp.width = bestWidth;
								lp.height = bestHeight;
								cv.setLayoutParams(lp);
								*/
								//cv.setLayoutParams(new RelativeLayout.LayoutParams(bestWidth, bestHeight));
						}
						mCamera.setParameters(param);	
						
						mCamera.setPreviewDisplay(holder);
					}catch(IOException e) {
						mCamera.release();
						mCamera = null;
					}
					//mCamera.startPreview();
				}
				
				public void surfaceChanged(SurfaceHolder holder, int format, int width,
						int height) {
					// TODO Auto-generated method stub
				//	Camera.Parameters param = mCamera.getParameters();
				//	param.setPictureFormat(PixelFormat.JPEG);
		
				//	param.setPreviewSize(width, height);
				//	param.setPictureSize(width, height);
					
				//	mCamera.setParameters(param);
					
		            Camera.Parameters param = mCamera.getParameters();  
		            param.setPictureFormat(PixelFormat.JPEG);
					int bestWidth = 1024;
					int bestHeight = 600;
					List<Camera.Size> sizeList = param.getSupportedPreviewSizes();
					String ssize=String.valueOf(sizeList.size());
					Log.i("surfaceChanged.............................................", ssize);
					//���sizeListֻ��һ������Ҳû�б�Ҫ��ʲô�ˣ���Ϊ����һ������ѡ��
					if(sizeList.size() > 1){
						Iterator<Camera.Size> itor = sizeList.iterator();
						while(itor.hasNext()){
							Camera.Size cur = itor.next();
							String swidth=String.valueOf(cur.width);
							String sheight=String.valueOf(cur.height);
							Log.i("surfaceChanged.............................................", swidth);
							Log.i("surfaceChanged.............................................", sheight);
							if(cur.width < bestWidth || cur.height < bestHeight ){
								bestWidth = cur.width;
								bestHeight = cur.height;
							}
						}
						//if(bestWidth!=1024){
						//	param.setPreviewSize(bestWidth, bestHeight);
							param.setPictureSize(bestWidth, bestHeight);
							//����ı���SIze�����ǻ�Ҫ����SurfaceView������Surface������ı��С������Camera��ͼ�������ܲ�
						//	fl.setLayoutParams(new LayoutParams(bestWidth, bestHeight));
							//cv.setLayoutParams(new LayoutParams(bestWidth, bestHeight));
							
						//}
							/*
							RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) cv.getLayoutParams();
							lp.width = bestWidth;
							lp.height = bestHeight;
							cv.setLayoutParams(lp);
							*/
							//cv.setLayoutParams(new RelativeLayout.LayoutParams(bestWidth, bestHeight));
					}
					mCamera.setParameters(param);		
					
					
					mCamera.startPreview();
				}
			});
			holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		}
    	
    }
}

