package com.nmlzju.roaming3d;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONException;

public class Hotspot {
	static public class Dimension3D{
		public float X;
		public float Y;
		public float Z;
		
		public Dimension3D(float X, float Y, float Z){
			this.X = X;
			this.Y = Y;
			this.Z = Z;
		}
	};
	
	static public class Dimension2D{
		public float X;
		public float Y;
		
		public Dimension2D(float X, float Y){
			this.X = X;
			this.Y = Y;
		}
	};
	
	static public class Resource{
		String name;
		String path;
		String description;
		
		public Resource(){
			
		}
		
		public Resource(String json){
			JSONArray value = null;
			try{
				value = new JSONArray(json);
			}catch(JSONException e){
				e.printStackTrace();
			}
			parse(value);
		}
		
		public Resource(JSONArray value){
			parse(value);
		}
			
		public void parse(JSONArray value){
			try{
				name = value.getString(0);
				path = value.getString(1);
				description = value.getString(2);
			}catch(JSONException e){
				e.printStackTrace();
			}
		}
	}
	
	static public class Image extends Resource{
		public Image(){
			super();
		}
		
		public Image(String json){
			super(json);
		}
		
		public Image(JSONArray value){
			super(value);
		}
	}
	
	static public class Media extends Resource{
		String thumbnail;
		
		public Media(){
			super();
		}
		
		public Media(String json){
			super(json);
		}
		
		public Media(JSONArray value){
			super(value);
		}
		
		public void parse(JSONArray value){
			super.parse(value);
			try {
				thumbnail = value.getString(3);
			} catch (JSONException e) {
				e.printStackTrace();
			}
		}
	}
	
	protected Dimension3D pos;
	protected Dimension2D size;
	protected String name;
	protected String description;
	protected LinkedList<Image> image;
	protected LinkedList<Media> media;

	public Hotspot(){
		image = new LinkedList<Image>();
		media = new LinkedList<Media>();
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
				addImage(new Image(images.getJSONArray(i)));
			}
	
			JSONArray media = value.getJSONArray(5);
			for(int i = 0; i < media.length(); i++){
				addMedia(new Media(media.getJSONArray(i)));
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

	public LinkedList<Image> getImage() {
		return image;
	}
	
	public List<Map<String,Object>> getImageModel() {
		List<Map<String,Object>> model = new LinkedList<Map<String,Object>>();
		
		Iterator<Image> iter = image.iterator();
		while(iter.hasNext()){
			Image img = iter.next();
			Map<String, Object> attr = new LinkedHashMap<String, Object>();
			attr.put("name", img.name);
			attr.put("path", img.path);
			attr.put("description", img.description);
			model.add(attr);
		}
		
		return model;
	}

	public void setImage(LinkedList<Image> image) {
		this.image = image;
	}

	public LinkedList<Media> getMedia() {
		return media;
	}
	
	public List<Map<String,Object>> getMediaModel() {
		List<Map<String,Object>> model = new LinkedList<Map<String,Object>>();
		
		Iterator<Media> iter = media.iterator();
		while(iter.hasNext()){
			Media md = iter.next();
			Map<String, Object> attr = new LinkedHashMap<String, Object>();
			attr.put("name", md.name);
			attr.put("path", md.path);
			attr.put("description", md.description);
			attr.put("thumbnail", md.thumbnail);
			model.add(attr);
		}
		
		return model;
	}

	public void setMedia(LinkedList<Media> media) {
		this.media = media;
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

	public void addImage(Image img){
		image.add(img);
	}
	
	public void removeImage(Image img){
		image.remove(img);
	}
	
	public void addMedia(Media md){
		media.add(md);
	}
	
	public void removeVideo(Media md){
		media.remove(md);
	}
}
