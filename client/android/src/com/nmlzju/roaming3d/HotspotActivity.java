package com.nmlzju.roaming3d;

import java.io.InputStream;
import java.util.*;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;

public class HotspotActivity extends FragmentActivity {

	/**
	 * The {@link android.support.v4.view.PagerAdapter} that will provide
	 * fragments for each of the sections. We use a
	 * {@link android.support.v4.app.FragmentPagerAdapter} derivative, which
	 * will keep every loaded fragment in memory. If this becomes too memory
	 * intensive, it may be best to switch to a
	 * {@link android.support.v4.app.FragmentStatePagerAdapter}.
	 */
	SectionsPagerAdapter mSectionsPagerAdapter;

	/**
	 * The {@link ViewPager} that will host the section contents.
	 */
	ViewPager mViewPager;
	
	String hotspot = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_hotspot);

		// Create the adapter that will return a fragment for each of the three
		// primary sections of the app.
		mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

		// Set up the ViewPager with the sections adapter.
		mViewPager = (ViewPager) findViewById(R.id.pager);
		mViewPager.setAdapter(mSectionsPagerAdapter);
		
		Intent intent = getIntent();
		hotspot = intent.getStringExtra("hotspot");
	}

	/**
	 * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
	 * one of the sections/tabs/pages.
	 */
	public class SectionsPagerAdapter extends FragmentPagerAdapter {

		public SectionsPagerAdapter(FragmentManager fm) {
			super(fm);
		}

		@Override
		public Fragment getItem(int position) {
			// getItem is called to instantiate the fragment for the given page.
			// Return a DummySectionFragment (defined as a static inner class
			// below) with the page number as its lone argument.
			Bundle args = new Bundle();
			args.putInt("position", position);
			args.putString("hotspot", hotspot);
			
			Fragment fragment = new DummySectionFragment();
			fragment.setArguments(args);
			return fragment;
		}

		@Override
		public int getCount() {
			// Show 3 total pages.
			return 3;
		}

		@Override
		public CharSequence getPageTitle(int position) {
			switch (position) {
			case 0:
				return getString(R.string.title_text);
			case 1:
				return getString(R.string.title_gallery);
			case 2:
				return getString(R.string.title_media);
			}
			return null;
		}
	}

	/**
	 * A dummy fragment representing a section of the app, but that simply
	 * displays dummy text.
	 */
	public static class DummySectionFragment extends Fragment {
		public DummySectionFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
			Bundle args = getArguments();
			int position = args.getInt("position");
			if(position < 0 || position > 2){
				position = 0;
			}
			
			Hotspot spot = new Hotspot(args.getString("hotspot"));

			switch(position){
				case 0:{
					View root_view = inflater.inflate(R.layout.fragment_hotspot_text, container, false);
					TextView title_view = (TextView) root_view.findViewById(R.id.section_title);
					TextView content_view = (TextView) root_view.findViewById(R.id.section_content);
					title_view.setText(spot.getName());
					content_view.setText(spot.getDescription());
					return root_view;
				}
				case 1:{
					HorizontalScrollView root_view = (HorizontalScrollView) inflater.inflate(R.layout.fragment_hotspot_gallery, container, false);
					LinearLayout layout = (LinearLayout) root_view.findViewById(R.id.hotspot_gallery);
					List<Hotspot.Image> images = spot.getImage();
					Iterator<Hotspot.Image> i = images.iterator();
					while(i.hasNext()){
						Hotspot.Image image = i.next();
						ImageView view = new ImageView(root_view.getContext());
						new DownloadImageTask(view).execute(image.path);
						layout.addView(view);
					}
					
					return root_view;
				}
				case 2:{
					ListView root_view = (ListView) inflater.inflate(R.layout.fragment_hotspot_media, container, false);
					List<Map<String, Object>> data = spot.getMediaModel();
					
					root_view.setAdapter(new MediaListAdapter(
						root_view.getContext(), data, R.layout.fragment_hotspot_media_item,
						new String[]{"name","path","description","thumbnail"},
						new int[]{R.id.hotspot_media_title, R.id.hotspot_media_path, R.id.hotspot_media_description, R.id.hotspot_media_thumbnail}
					));
					
					root_view.setOnItemClickListener(new OnItemClickListener(){
						public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
							TextView url_view = (TextView) view.findViewById(R.id.hotspot_media_path);
							if(url_view != null){
								Intent browser = new Intent(Intent.ACTION_VIEW, Uri.parse(url_view.getText().toString()));
								startActivity(browser);
							}
						}
					});

					return root_view;
				}
			}
			return null;
		}
	}
	
	private static class MediaListAdapter extends SimpleAdapter{

		public MediaListAdapter(Context context, List<? extends Map<String, ?>> data, int resource, String[] from, int[] to) {
			super(context, data, resource, from, to);
		}
		
		@Override
		public void setViewImage(ImageView view, String path){
			if(path.startsWith("http://") || path.startsWith("https://")){
				new DownloadImageTask(view).execute(path);
			}else{
				super.setViewImage(view, path);
			}
		}
		
	}
	
	private static class DownloadImageTask extends AsyncTask<String, Void, Bitmap> {
		ImageView view;

		public DownloadImageTask(ImageView image_view) {
			this.view = image_view;
		}

		protected Bitmap doInBackground(String... urls) {
			String urldisplay = urls[0];
			Bitmap bitmap = null;
			
			try {
				InputStream in = new java.net.URL(urldisplay).openStream();
				bitmap = BitmapFactory.decodeStream(in);
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return bitmap;
		}

		protected void onPostExecute(Bitmap result) {
			view.setImageBitmap(result);
		}
	}
}
