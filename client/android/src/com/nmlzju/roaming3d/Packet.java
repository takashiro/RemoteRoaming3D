package com.nmlzju.roaming3d;
import org.json.*;

public class Packet {
	enum Command{
		INVALID,

		//Server to Client
		UPDATE_VIDEO_FRAME,
		MAKE_TOAST_TEXT,
		ENTER_HOTSPOT,
		QUIT,

		//Client to Server
		CREATE_DEVICE,
		ROTATE_CAMERA,
		SCALE_CAMERA,
		MOVE_CAMERA,
		
		CONTROL_HOTSPOTS,
		DOUBLE_CLICK,
		
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
			if(!raw.isNull(1)){
				packet.args = raw.getJSONArray(1);
			}
			return packet;
		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return null;
	}
	
	enum Message{
		SERVER_REACHES_MAX_CLIENT_NUM,
		SERVER_IS_TO_BE_CLOSED
	};
	
	static int[] Message2Toast = new int[Message.values().length];
	static{
		Message2Toast[Message.SERVER_REACHES_MAX_CLIENT_NUM.ordinal()] = R.string.toast_server_reaches_max_client_num;
	}
}
