package com.example.testcluster;

import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.util.Log;


public class FileUpload {
	
	//private static String newName = "fcsjt.jpg";
	//private static String uploadFile = "/sdcard/fcsjt.jpg";

	/* �ϴ��ļ���Server�ķ��� */
	public static String uploadFile(String uploadFile,String servelet,String id) {
		String realUrl = new String();
		if(servelet.equals("image"))realUrl = HttpUtil.BASE_URL + "/AddImage";
		else if(servelet.equals("audio"))realUrl = HttpUtil.BASE_URL + "/AddAudio";
		else if(servelet.equals("video"))realUrl = HttpUtil.BASE_URL + "/AddVideo";
		else if(servelet.equals("upload"))realUrl = HttpUtil.BASE_URL + "/Upload";
		else return "0";
		
		String newName = uploadFile.substring(uploadFile.lastIndexOf("/") + 1);
		String end = "\r\n";
		String twoHyphens = "--";
		String boundary = "*****";
		String path = "";
		try {
			URL url = new URL(realUrl);
			HttpURLConnection con = (HttpURLConnection) url.openConnection();
			/* ����Input��Output����ʹ��Cache */
			con.setDoInput(true);
			con.setDoOutput(true);
			con.setUseCaches(false);
			/* ���ô��͵�method=POST */
			con.setRequestMethod("POST");
			/* setRequestProperty */
			con.setRequestProperty("Connection", "Keep-Alive");
			con.setRequestProperty("Charset", "UTF-8");
			con.setRequestProperty("Content-Type",
					"multipart/form-data;boundary=" + boundary);
			/* ����DataOutputStream */
			DataOutputStream ds = new DataOutputStream(con.getOutputStream());
			
			ds.writeBytes(twoHyphens + boundary + end);
			ds.writeBytes("Content-Disposition: form-data; "
					+ "name=\"id\"" + end + end + id);
			ds.writeBytes(end);
			
			ds.writeBytes(twoHyphens + boundary + end);
			ds.writeBytes("Content-Disposition: form-data; "
					+ "name=\"file1\";filename=\"" + newName + "\"" + end);
			ds.writeBytes(end);
			
			/* ȡ���ļ���FileInputStream */
			FileInputStream fStream = new FileInputStream(uploadFile);
			/* ����ÿ��д��1024bytes */
			int bufferSize = 1024;
			byte[] buffer = new byte[bufferSize];

			int length = -1;
			/* ���ļ���ȡ����������� */
			while ((length = fStream.read(buffer)) != -1) {
				/* ������д��DataOutputStream�� */
				ds.write(buffer, 0, length);
			}
			ds.writeBytes(end);
			ds.writeBytes(twoHyphens + boundary + twoHyphens + end);

			/* close streams */
			fStream.close();
			ds.flush();

			Log.i("rrrr", "rrrr up succeed");
			/* ȡ��Response���� */
			InputStream is = con.getInputStream();
			InputStreamReader isr = new InputStreamReader(is, "UTF-8"); 
			int ch;
			StringBuffer b = new StringBuffer();
			while ((ch = isr.read()) != -1) {
				b.append((char) ch);
			}
			/* ��Response��ʾ��Dialog */
			//showDialog("�ϴ��ɹ�" + b.toString().trim());
			/* �ر�DataOutputStream */
			path = b.toString();
			ds.close();
		} catch (Exception e) {
			//showDialog("�ϴ�ʧ��" + e);
			Log.i("fileup", "fileup" + e.toString());
		}
		
		return path;
	}

}
