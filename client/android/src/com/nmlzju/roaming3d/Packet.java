package com.nmlzju.roaming3d;
import org.json.*;

public class Packet {
	enum Command{
		INVALID,

		//Server to Client
		UPDATE_VIDEO_FRAME,

		//Client to Server
		SET_RESOLUTION,
		ROTATE_CAMERA,
		SCALE_CAMERA,
		MOVE_CAMERA,
		
		length
	};
	
	public Command command;
	public JSONArray args = new JSONArray();
	
	public Packet(){
		this.command = Command.INVALID;
	}
	
	public Packet(Command command){
		this.command = command;
	}
	
	public String toString(){
		JSONArray packet = new JSONArray();
		packet.put(this.command.ordinal());
		packet.put(this.args);
		return packet.toString();
	}
	
	public static Packet parse(String json){
		if(json == null || json.length() <= 0){
			return null;
		}
		
		try {
			JSONArray raw = new JSONArray(json);
			Packet packet = new Packet();
			packet.command = Command.values()[raw.getInt(0)];
			packet.args = raw.getJSONArray(1);
			return packet;
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return null;
	}
}
