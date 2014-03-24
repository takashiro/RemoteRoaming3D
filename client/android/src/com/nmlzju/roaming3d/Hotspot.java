package com.nmlzju.roaming3d;

import java.util.LinkedList;

import org.json.JSONArray;
import org.json.JSONException;

public class Hotspot {
	static class Dimension3D{
		public float X;
		public float Y;
		public float Z;
		
		public Dimension3D(float X, float Y, float Z){
			this.X = X;
			this.Y = Y;
			this.Z = Z;
		}
	};
	
	static class Dimension2D{
		public float X;
		public float Y;
		
		public Dimension2D(float X, float Y){
			this.X = X;
			this.Y = Y;
		}
	};
	
	protected Dimension3D pos;
	protected Dimension2D size;
	protected String name;
	protected String description;
	protected LinkedList<String> image;
	protected LinkedList<String> audio;
	protected LinkedList<String> video;

	public Hotspot(){
		image = new LinkedList<String>();
		audio = new LinkedList<String>();
		video = new LinkedList<String>();
	}
	
	public Hotspot(String json){
		this();
		parse(json);
	}
	
	public Hotspot(JSONArray value){
		this();
		parse(value);
	}
	
		
	public void parse(String json){
		try{
			parse(new JSONArray(json));
		}catch(JSONException e){
			e.printStackTrace();
		}
	}
		
	public void parse(JSONArray value){
		try{
			setName(value.getString(0));
			
			JSONArray pos = value.getJSONArray(1);
			setPos(new Dimension3D((float) pos.getDouble(0), (float) pos.getDouble(1), (float) pos.getDouble(2)));
			
			JSONArray size = value.getJSONArray(2);
			setSize(new Dimension2D((float) size.getDouble(0), (float) size.getDouble(1)));
			setDescription(value.getString(3));
	
			JSONArray images = value.getJSONArray(4);
			for(int i = 0; i < images.length(); i++){
				addImage(images.getString(i));
			}
	
			JSONArray audios = value.getJSONArray(5);
			for(int i = 0; i < audios.length(); i++){
				addAudio(audios.getString(i));
			}
	
			JSONArray videos = value.getJSONArray(6);
			for(int i = 0; i < videos.length(); i++){
				addVideo(videos.getString(i));
			}
		}catch(JSONException e){
			e.printStackTrace();
		}
	}

	public Dimension3D getPos() {
		return pos;
	}

	public void setPos(Dimension3D pos) {
		this.pos = pos;
	}

	public Dimension2D getSize() {
		return size;
	}

	public void setSize(Dimension2D size) {
		this.size = size;
	}

	public LinkedList<String> getImage() {
		return image;
	}

	public void setImage(LinkedList<String> image) {
		this.image = image;
	}

	public LinkedList<String> getAudio() {
		return audio;
	}

	public void setAudio(LinkedList<String> audio) {
		this.audio = audio;
	}

	public LinkedList<String> getVideo() {
		return video;
	}

	public void setVideo(LinkedList<String> video) {
		this.video = video;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getName() {
		return name;
	}
	
	public void setDescription(String description) {
		this.description = description;
	}

	public String getDescription() {
		return description;
	}


	public void addImage(String path){
		image.add(path);
	}
	
	public void removeImage(String path){
		image.remove(path);
	}
	
	public void addAudio(String path){
		audio.add(path);
	}
	
	public void removeAudio(String path){
		audio.remove(path);
	}
	
	public void addVideo(String path){
		video.add(path);
	}
	
	public void removeVideo(String path){
		video.remove(path);
	}
}
