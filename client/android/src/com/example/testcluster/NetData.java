package com.example.testcluster;

public class NetData {
	
	private float flag;
	private float deltaX;
	private float deltaY;
	
	public NetData(float f, float x, float y) {
		// TODO Auto-generated constructor stub
		flag = f;
		deltaX = x;
		deltaY = y;
	}
	
	public Integer getF(){
		return new Integer((int)flag);
	}
	
	public Integer getX(){
		return new Integer((int)deltaX);
	}
	
	public Integer getY(){
		return new Integer((int)deltaY);
	}

}
